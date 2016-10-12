// GPU task base class
// (c) EB Nov 2009

#include "GPUTask.h"

GPUTask::GPUTask(const char * programFile,Logger * log)
{
  cl::Context * c = 0;
  cl::CommandQueue * q = 0;
  cl::Program * p = 0;
  bool ok = true;
  std::string s;
  std::vector<unsigned char> binary;

  if (log == 0) { ok = false; goto END; }
  c = cl::Context::create();
  if (c == 0) { log->append("Context creation failed"); ok = false; goto END; }
  q = c->createCommandQueue(0,0);
  if (q == 0) { log->append("Command queue creation failed"); ok = false; goto END; }
  if (programFile == 0) p = 0;
  else
  {
#ifdef WIN32
    // We run from the vs2008 solution directory, so we must
    // change the path... (quick & dirty patch)
    char programFileWin[1000];
    _snprintf(programFileWin,1000,"../%s",programFile);
    p = c->createProgramWithFile(programFileWin);
#else
    p = c->createProgramWithFile(programFile);
#endif
    if (p == 0) { log->append("Program creation failed"); ok = false; goto END; }
  }

END:
  if (!ok)
  {
    if (p != 0) { delete p; p = 0; }
    if (q != 0) { delete q; q = 0; }
    if (c != 0) { delete c; c = 0; }
  }
  mProgram = p;
  mQueue = q;
  mContext = c;
}

GPUTask::~GPUTask()
{
  delete mProgram;
  delete mQueue;
  delete mContext;
}

bool GPUTask::buildProgram(const char * options,Logger * log)
{
  if (mProgram == 0) return false; // Invalid
  if (options == 0 || log == 0) return false; // Invalid

  std::string s;
  bool ok = mProgram->build(options,s);
  if (!ok) { log->append("Build failed, errors:"); log->append(s.c_str()); }
#if 0 // Show PTX code (nvidia only)
  ok &= p->getBinary(binary);
  if (!ok) { log->append("Get binary failed"); goto END; }
  log->append((char *)&(binary[0]));
#endif
  return ok;
}

cl::Kernel * GPUTask::createKernel(const char * name)
{
  if (mProgram == 0) return 0; // Invalid
  return mProgram->createKernel(name);
}
