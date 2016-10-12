// Arithmetic tasks
// (c) EB Nov 2009

#ifndef ArithmeticTasks_h
#define ArithmeticTasks_h

#include "Config.h"
#include "GPUTask.h"
#include "CPUTask.h"

enum AddNVariants
{
  ADDN_V1 = 0,
  ADDN_V2,
  ADDN_V3,
  NB_ADDN_VARIANTS
};

enum Mul1Variants
{
  MUL1_V1 = 0,
  NB_MUL1_VARIANTS
};


// Sum two buffers
class AddNGPUTask : public GPUTask
{

public:

  AddNGPUTask(int variant,int wordSize,Logger * log) : GPUTask("gpu_add.cl",log)
  {
    mWS = wordSize;
    mVariant = variant;
    char options[200];
    _snprintf(options,200,"-DWORD_SIZE=%d",mWS);
    mBuildOK = buildProgram(options,log);
    if (!mBuildOK) fprintf(stderr,"Build failed\n");
  }

  double run(int workgroupSize,size_t sz);

private:

  // Word size
  int mWS;
  // Variant
  int mVariant;
  // Build OK?
  bool mBuildOK;

};

// Multiply buffer by 1 digit
class Mul1GPUTask : public GPUTask
{

public:

  Mul1GPUTask(int variant,int blockSize,Logger * log) : GPUTask("gpu_mul1.cl",log)
  {
    mBS = blockSize;
    mVariant = variant;
    char options[200];
    const int logBase = 30;
    const int base = 1<<logBase;
    const int baseMinus1 = base-1;
    _snprintf(options,200,"-DLOG_BASE=%d -DBASE=%d -DBASE_MINUS1=%d -DBLOCK_SIZE=%d",logBase,base,baseMinus1,blockSize);
    mBuildOK = buildProgram(options,log);
  }

  double run(int workgroupSize,size_t sz);

private:

  // Build OK?
  bool mBuildOK;
  // Block size
  int mBS;
  // Variant
  int mVariant;

};

// CPU

class AddNCPUTask : public CPUTask
{
public:
  AddNCPUTask(int variant,int wordSize) : mWS(wordSize), mVariant(variant) { }

  double run(int nThreads,size_t sz);

private:

  // Word size (32 or 64)
  int mWS;
  // Variant
  int mVariant;
};

#endif // ArithmeticTasks_h
