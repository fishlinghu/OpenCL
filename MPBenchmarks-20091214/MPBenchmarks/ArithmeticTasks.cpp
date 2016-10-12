// Arithmetic tasks
// (c) EB Nov 2009

#include <stdlib.h>
#include "ArithmeticTasks.h"

#ifdef Linux
typedef short __int16;
typedef int __int32;
typedef long long int __int64;
#endif

// Get the ratio of user bits per stored bits for a given word size
// (constants defined in OpenCL code)
inline double useRatio(int ws)
{
  switch (ws)
  {
  case 16: return 14.0/16.0;
  case 32: return 30.0/32.0;
  case 64: return 62.0/64.0;
  }
  return 1.0;
}

double AddNGPUTask::run(int workgroupSize,size_t sz)
{
  if (!mBuildOK) return -1;

  cl::Context * c = getContext();
  cl::CommandQueue * q = getQueue();
  cl::Program * p = getProgram();
  if (c == 0 || q == 0 || p == 0) return -1;

  // Check allocation size
  size_t max_sz = c->getDeviceMaxMemAllocSize(); // Max global mem size
  if (3*sz > max_sz) return -1;

  cl::Buffer * a = c->createBuffer(CL_MEM_READ_WRITE,sz);
  cl::Buffer * b = c->createBuffer(CL_MEM_READ_WRITE,sz);
  cl::Buffer * y = c->createBuffer(CL_MEM_READ_WRITE,sz);
  cl::Kernel * kernel = 0;
  if (mVariant == ADDN_V1) kernel = p->createKernel("add_v1");
  else if (mVariant == ADDN_V2) kernel = p->createKernel("add_v2");
  unsigned char * buf = (unsigned char *)_aligned_malloc(sz,16);
  int n = (int)sz / (mWS>>3); // N words in SZ
  double mbps = -1;
  if (kernel == 0 || a == 0 || b == 0 || y == 0 || buf == 0) goto END;

  // Initialize A and B
  for (size_t i=0;i<sz;i++) buf[i] = (unsigned char)(rand() & 0xFF);
  if (!q->writeBuffer(a,true,0,sz,buf).isValid()) goto END;
  for (size_t i=0;i<sz;i++) buf[i] = (unsigned char)(rand() & 0xFF);
  if (!q->writeBuffer(b,true,0,sz,buf).isValid()) goto END;

  // Run tests, double nOps until min time is reached
  kernel->setArg(0,a);
  kernel->setArg(1,b);
  kernel->setArg(2,y);
  if (!q->execKernel1(kernel,n,workgroupSize).isValid()) goto END;
  for (int nOps = 1; ; nOps <<= 1)
  {
    double t0 = getT();
    for (int i=0;i<nOps;i++) q->execKernel1(kernel,n,workgroupSize);
    q->finish();
    double t = (getT() - t0);
    if (t < MIN_RUNNING_TIME) continue;
    // OK, t is large enough
    t /= (double)nOps;
    mbps = (double)sz*useRatio(mWS)*1.0e-6/t; // MB/s
    break;
  }

END: // Cleanup
  if (buf != 0) _aligned_free(buf);
  if (a != 0) delete a;
  if (b != 0) delete b;
  if (y != 0) delete y;
  if (kernel != 0) delete kernel;

  return mbps;
}

double Mul1GPUTask::run(int workgroupSize,size_t sz)
{
  if (!mBuildOK) return -1;

  cl::Context * c = getContext();
  cl::CommandQueue * q = getQueue();
  cl::Program * p = getProgram();
  if (c == 0 || q == 0 || p == 0) return -1;

  // Check allocation size
  size_t max_sz = c->getDeviceMaxMemAllocSize(); // Max global mem size
  if (2*sz > max_sz) return -1;

  cl::Buffer * a = c->createBuffer(CL_MEM_READ_WRITE,sz);
  cl::Buffer * y = c->createBuffer(CL_MEM_READ_WRITE,sz);
  cl::Kernel * kernel = 0;
  if (mVariant == MUL1_V1) kernel = p->createKernel("mul1_v1");

  unsigned char * buf = (unsigned char *)_aligned_malloc(sz,16);
  int n = (int)sz>>2; // 4 bytes/word
  const int kk = 0x2FEFEFEF;
  double mbps = -1;
  if (kernel == 0 || a == 0 || y == 0 || buf == 0) goto END;

  // Initialize A
  for (size_t i=0;i<sz;i++) buf[i] = (unsigned char)(rand() & 0xFF);
  if (!q->writeBuffer(a,true,0,sz,buf).isValid()) goto END;

  // Run tests, double nOps until min time is reached
  kernel->setArg(0,kk);
  kernel->setArg(1,a);
  kernel->setArg(2,y);
  if (!q->execKernel1(kernel,n,workgroupSize).isValid()) goto END;
  for (int nOps = 1; ; nOps <<= 1)
  {
    double t0 = getT();
    for (int i=0;i<nOps;i++) q->execKernel1(kernel,n,workgroupSize);
    q->finish();
    double t = (getT() - t0);
    if (t < MIN_RUNNING_TIME) continue;
    // OK, t is large enough
    t /= (double)nOps;
    mbps = (double)sz*useRatio(32)*1.0e-6/t; // MB/s
    break;
  }

END: // Cleanup
  if (buf != 0) _aligned_free(buf);
  if (a != 0) delete a;
  if (y != 0) delete y;
  if (kernel != 0) delete kernel;

  return mbps;
}

// CPU

struct AddNThreadParam
{
  int nOps; // number of loops
  int index; // Block index
  size_t sz; // size to process in thread
  int wordSize; // 32 or 64
  int variant; // ADD_V1, ADD_V2
  const unsigned char * a;
  const unsigned char * b;
  unsigned char * out;
};

template <typename T,int LOG_BASE> void addNv1(AddNThreadParam * p)
{
  size_t n = p->sz / sizeof(T); // Words in block
  const T * a = (const T *)(p->a);
  const T * b = (const T *)(p->b);
  T * out = (T *)(p->out);
  T BASE = (T)1<<(T)LOG_BASE;
  T BASE_MINUS1 = BASE - (T)1;

  for (int it=0;it<p->nOps;it++)
  {
    T t = 0; // "carry" from previous word
    if (p->index > 0)
    {
      // Get "carry" for last values of previous block
      t = (a[-1] + b[-1]) >> (T)LOG_BASE;
    }

    for (size_t i=0;i<n;i++)
    {
      T s = a[i] + b[i];
      out[i] = (s & BASE_MINUS1) + t;
      t = s >> (T)LOG_BASE;
    }
  }
}

template <typename T,int LOG_BASE> void addNv2(AddNThreadParam * p)
{
  size_t n = p->sz / sizeof(T); // Words in block
  const T * a = (const T *)(p->a);
  const T * b = (const T *)(p->b);
  T * out = (T *)(p->out);

  for (int it=0;it<p->nOps;it++)
  {
    for (size_t i=0;i<n;i++)
    {
      out[i] = a[i] + b[i];
    }
  }
}

#ifndef Linux // Uses Win32 intrinsics
void addNv3(AddNThreadParam * p)
{
  const int LOG_BASE = 30;
  const int BASE = 1<<LOG_BASE;
  const int BASE_MINUS1 = BASE - 1;

  size_t n = p->sz >> 4; // We process blocks of 16 bytes
  const __m128i * a = (const __m128i *)(p->a);
  const __m128i * b = (const __m128i *)(p->b);
  __m128i * out = (__m128i *)(p->out);

  __m128i t;
  __m128i mask = _mm_set1_epi32(BASE_MINUS1);
  for (int it=0;it<p->nOps;it++)
  {
    // TODO: initialize T with the carry from the end of previous block
    t = _mm_setzero_si128();
    for (size_t i=0;i<n;i++)
    {
      __m128i s = _mm_add_epi32(_mm_load_si128(a+i),_mm_load_si128(b+i));
      __m128i o = _mm_add_epi32(_mm_and_si128(s,mask),t);
      _mm_store_si128(out+i,o);
      t = _mm_srli_epi32(s,LOG_BASE);
    }
  }
}
#endif

thread_return_t AddNThread(void * x)
{
  AddNThreadParam * p = (AddNThreadParam *)x;

  if (p->wordSize == 16 && p->variant == ADDN_V1) addNv1<__int16,14>(p);
  else if (p->wordSize == 32 && p->variant == ADDN_V1) addNv1<__int32,30>(p);
  else if (p->wordSize == 64 && p->variant == ADDN_V1) addNv1<__int64,62>(p);
  else if (p->wordSize == 16 && p->variant == ADDN_V2) addNv2<__int16,14>(p);
  else if (p->wordSize == 32 && p->variant == ADDN_V2) addNv2<__int32,30>(p);
  else if (p->wordSize == 64 && p->variant == ADDN_V2) addNv2<__int64,62>(p);
#ifndef Linux
  else if (p->wordSize == 32 && p->variant == ADDN_V3) addNv3(p);
#endif

  return 0;
}

double AddNCPUTask::run(int nThreads,size_t sz)
{
  if (nThreads <= 0 || sz <= 0) return -1; // invalid
  size_t sz1 = sz / nThreads;
  if (sz1 <= 0) return -1; // SZ too small

  unsigned char * in_a = (unsigned char *)_aligned_malloc(sz,ALLOC_ALIGN);
  unsigned char * in_b = (unsigned char *)_aligned_malloc(sz,ALLOC_ALIGN);
  unsigned char * out = (unsigned char *)_aligned_malloc(sz,ALLOC_ALIGN);
  double mbps = -1;

  std::vector<AddNThreadParam> params(nThreads);
  for (int nOps=1; ;nOps<<=1)
  {
    // Initialize memory
    for (size_t i=0;i<sz;i++)
    {
      in_a[i] = (unsigned char)(i & 0xFF);
      in_b[i] = (unsigned char)((i+99) & 0xFF);
      out[i] = 0;
    }
    // setup params
    for (int i=0;i<nThreads;i++)
    {
      params[i].a = in_a + i*sz1;
      params[i].b = in_b + i*sz1;
      params[i].out = out + i*sz1;
      params[i].sz = sz1;
      params[i].nOps = nOps;
      params[i].wordSize = mWS;
      params[i].variant = mVariant;
      params[i].index = i;
    }
    // run threads
    double t = runCPUThreads(params,AddNThread);
    // Check result
    if (nOps == 1)
    {
      // ZZZ: do it!
    }

    if (t < MIN_RUNNING_TIME) continue; // Too short
    t /= (double)nOps;

    mbps = (double)sz1*(double)nThreads*useRatio(mWS)*1.0e-6/t;
    break;
  } // nOps loop

  _aligned_free(in_a);
  _aligned_free(in_b);
  _aligned_free(out);
  return mbps;
}
