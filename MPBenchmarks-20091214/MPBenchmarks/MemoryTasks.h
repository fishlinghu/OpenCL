// Memory tasks
// (c) EB Nov 2009

#ifndef MemoryTasks_h
#define MemoryTasks_h

#include "Config.h"
#include "GPUTask.h"
#include "CPUTask.h"

// Copy two buffers (GPU)
class CopyGPUTask : public GPUTask
{

public:

  enum CopyType
  {
    HOST_TO_DEVICE_COPY,
    DEVICE_TO_HOST_COPY,
    DEVICE_TO_DEVICE_COPY
  };

  CopyGPUTask(int copyType,Logger * log) : GPUTask(0,log)
  {
    mCT = copyType;
  }

  ~CopyGPUTask()
  {
  }

  double run(int workgroupSize,size_t sz);

private:

  // Copy type
  int mCT;

};

// Zero one buffer (GPU)
class ZeroGPUTask : public GPUTask
{

public:

  ZeroGPUTask(int wordSize,Logger * log) : GPUTask("gpu_zero.cl",log)
  {
    mWS = wordSize;
    char options[200];
    _snprintf(options,200,"-DWORD_SIZE=%d",mWS);
    buildProgram(options,log);
  }

  ~ZeroGPUTask()
  {
  }

  double run(int workgroupSize,size_t sz);

private:

  // Word size
  int mWS;

};

// CPU

class CopyCPUTask : public CPUTask
{
public:
  double run(int nThreads,size_t sz);
};

class ZeroCPUTask : public CPUTask
{
public:
  double run(int nThreads,size_t sz);
};

#endif // MemoryTasks_h
