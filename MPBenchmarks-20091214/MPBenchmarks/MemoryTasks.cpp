// Memory tasks
// (c) EB Nov 2009

#include "MemoryTasks.h"

#include <string.h>


// GPU

double CopyGPUTask::run(int workgroupSize,size_t sz)
{
  cl::Context * c = getContext();
  cl::CommandQueue * q = getQueue();
  if (c == 0 || q == 0) return -1;

  // Check allocation size
  size_t max_sz = c->getDeviceMaxMemAllocSize(); // Max global mem size
  if (mCT == DEVICE_TO_DEVICE_COPY)
  {
    if (2*sz > max_sz) return -1;
  }
  if (sz > max_sz) return -1;

  bool ok = true;
  cl::Buffer * a = c->createBuffer(CL_MEM_READ_WRITE,sz);
  if (a == 0) ok = false;
  cl::Buffer * b = 0;
  if (mCT == DEVICE_TO_DEVICE_COPY)
  {
    b = c->createBuffer(CL_MEM_READ_WRITE,sz);
    if (b == 0) ok = false;
  }
  unsigned char * buf = (unsigned char *)_aligned_malloc(sz,16);
  if (buf == 0) ok = false;
  double mbps = -1;

  if (!ok) goto END; // Alloc failed

  // Initialize A and check errors
  for (size_t i=0;i<sz;i++) buf[i] = (unsigned char)(i & 0xFF);
  if (!q->writeBuffer(a,true,0,sz,buf).isValid()) goto END;
  for (size_t i=0;i<sz;i++) buf[i] = (unsigned char)0;
  if (!q->readBuffer(a,true,0,sz,buf).isValid()) goto END;
  ok = true;

  // check write+read loop
  for (size_t i=0;i<sz;i++) if (buf[i] != (unsigned char)(i & 0xFF)) ok = false;
  if (!ok) { fprintf(stderr,"write+read failed\n"); goto END; }

  // check write+copy+read loop
  if (b != 0)
  {
    for (size_t i=0;i<sz;i++) buf[i] = (unsigned char)(i & 0xFF);
    if (!q->writeBuffer(a,true,0,sz,buf).isValid()) goto END;
    for (size_t i=0;i<sz;i++) buf[i] = (unsigned char)0;
    if (!q->writeBuffer(b,true,0,sz,buf).isValid()) goto END;
    if (!q->copyBuffer(a,b,0,0,sz).isValid()) goto END;
    if (!q->readBuffer(b,true,0,sz,buf).isValid()) goto END;
    for (size_t i=0;i<sz;i++) if (buf[i] != (unsigned char)(i & 0xFF)) ok = false;
    if (!ok) { fprintf(stderr,"write+copy+read failed\n"); goto END; }
  }

  // Run tests, double nOps until the min time is reached
  for (int nOps = 5; ; nOps <<= 1)
  {
    double t0 = getT();
    switch (mCT)
    {
    case HOST_TO_DEVICE_COPY:
      for (int i=0;i<nOps;i++) q->writeBuffer(a,false,0,sz,buf);
      break;
    case DEVICE_TO_HOST_COPY:
      for (int i=0;i<nOps;i++) q->readBuffer(a,false,0,sz,buf);
      break;
    case DEVICE_TO_DEVICE_COPY:
      for (int i=0;i<nOps;i++) q->copyBuffer(a,b,0,0,sz);
      break;
    }
    q->finish();
    double t = (getT() - t0);
    if (t < MIN_RUNNING_TIME) continue;
    // OK, t is large enough
    t /= (double)nOps;
    mbps = (double)sz*1.0e-6/t; // MB/s
    break;
  }

END:
  if (buf != 0) _aligned_free(buf);
  if (a != 0) delete a;
  if (b != 0) delete b;

  return mbps;
}

double ZeroGPUTask::run(int workgroupSize,size_t sz)
{
  cl::Context * c = getContext();
  cl::CommandQueue * q = getQueue();
  cl::Program * p = getProgram();
  if (c == 0 || q == 0 || p == 0) return -1;

  // Check allocation size
  size_t max_sz = c->getDeviceMaxMemAllocSize(); // Max global mem size
  if (sz > max_sz) return -1;

  cl::Buffer * a = c->createBuffer(CL_MEM_READ_WRITE,sz);
  unsigned char * buf = (unsigned char *)_aligned_malloc(sz,16);
  cl::Kernel * kernel = p->createKernel("zero");
  double mbps = -1;
  int n = (int)sz / (mWS>>3); // N words in SZ
  bool ok;
  if (kernel == 0 || a == 0 || buf == 0) goto END;

  // Initialize A
  for (size_t i=0;i<sz;i++) buf[i] = (unsigned char)(i & 0xFF);
  if (!q->writeBuffer(a,true,0,sz,buf).isValid()) goto END;

  // Run tests, double nOps until min time is reached
  kernel->setArg(0,a);
  if (!q->execKernel1(kernel,n,workgroupSize).isValid()) goto END;

  // check write+zero+read
  if (!q->readBuffer(a,true,0,sz,buf).isValid()) goto END;
  ok = true;
  for (size_t i=0;i<sz;i++) if (buf[i] != (unsigned char)0) ok = false;
  if (!ok) { fprintf(stderr,"write+zero+read failed\n"); goto END; }

  for (int nOps = 5; ; nOps <<= 1)
  {
    double t0 = getT();
    for (int i=0;i<nOps;i++) q->execKernel1(kernel,n,workgroupSize);
    q->finish();
    double t = (getT() - t0);
    if (t < MIN_RUNNING_TIME) continue;
    // OK, t is large enough
    t /= (double)nOps;
    mbps = (double)sz*1.0e-6/t; // MB/s
    break;
  }

END:
  if (buf != 0) _aligned_free(buf);
  if (a != 0) delete a;
  if (kernel != 0) delete kernel;

  return mbps;
}

// CPU

struct CopyThreadParam
{
  int nOps; // number of loops
  size_t sz; // size to process in thread
  const unsigned char * in;
  unsigned char * out;
};

thread_return_t CopyThread(void * x)
{
  CopyThreadParam * p = (CopyThreadParam *)x;
#ifdef WIN32
  for (int i=0;i<p->nOps;i++) CopyMemory(p->out,p->in,p->sz);
#else
  for (int i=0;i<p->nOps;i++) memcpy(p->out,p->in,p->sz);
#endif
  return 0;
}

double CopyCPUTask::run(int nThreads,size_t sz)
{
  if (nThreads <= 0 || sz <= 0) return -1; // invalid
  size_t sz1 = sz / nThreads;
  if (sz1 <= 0) return -1; // SZ too small

  unsigned char * in = (unsigned char *)_aligned_malloc(sz,ALLOC_ALIGN);
  unsigned char * out = (unsigned char *)_aligned_malloc(sz,ALLOC_ALIGN);
  double mbps = -1;

  std::vector<CopyThreadParam> params(nThreads);
  for (int nOps=1; ;nOps<<=1)
  {
    // Initialize memory
    for (size_t i=0;i<sz;i++)
    {
      in[i] = (unsigned char)(i & 0xFF);
      out[i] = 0;
    }
    // setup params
    for (int i=0;i<nThreads;i++)
    {
      params[i].in = in + i*sz1;
      params[i].out = out + i*sz1;
      params[i].sz = sz1;
      params[i].nOps = nOps;
    }
    // run threads
    double t = runCPUThreads(params,CopyThread);
    // Check result
    if (nOps == 1)
    {
      if (memcmp(in,out,sz) != 0) { fprintf(stderr,"CPU copy error\n"); mbps = -1; break; }
    }

    if (t < MIN_RUNNING_TIME) continue; // Too short
    t /= (double)nOps;

    mbps = (double)sz1*(double)nThreads*1.0e-6/t;
    break;
  }

  _aligned_free(in);
  _aligned_free(out);
  return mbps;
}

struct ZeroThreadParam
{
  int nOps; // number of loops
  size_t sz; // size to process in thread
  unsigned char * out;
};

thread_return_t ZeroThread(void * x)
{
  CopyThreadParam * p = (CopyThreadParam *)x;
#ifdef WIN32
  for (int i=0;i<p->nOps;i++) ZeroMemory(p->out,p->sz);
#else
  for (int i=0;i<p->nOps;i++) memset(p->out,0,p->sz);
#endif
  return 0;
}

double ZeroCPUTask::run(int nThreads,size_t sz)
{
  if (nThreads <= 0 || sz <= 0) return -1; // invalid
  size_t sz1 = sz / nThreads;
  if (sz1 <= 0) return -1; // SZ too small

  unsigned char * out = (unsigned char *)_aligned_malloc(sz,ALLOC_ALIGN);
  double mbps = -1;

  std::vector<CopyThreadParam> params(nThreads);
  for (int nOps=1; ;nOps<<=1)
  {
    // Initialize memory
    for (size_t i=0;i<sz;i++)
    {
      out[i] = (unsigned char)(i & 0xFF);
    }
    // setup params
    for (int i=0;i<nThreads;i++)
    {
      params[i].out = out + i*sz1;
      params[i].sz = sz1;
      params[i].nOps = nOps;
    }
    // run threads
    double t = runCPUThreads(params,ZeroThread);
    // Check result
    if (nOps == 1)
    {
      bool ok = true;
      for (size_t i=0;i<sz;i++) if ( out[i] != 0 ) { ok = false; break; }
      if (!ok) { fprintf(stderr,"CPU zero error\n"); mbps = -1; break; }
    }

    if (t < MIN_RUNNING_TIME) continue; // Too short
    t /= (double)nOps;

    mbps = (double)sz1*(double)nThreads*1.0e-6/t;
    break;
  }

  _aligned_free(out);
  return mbps;
}
