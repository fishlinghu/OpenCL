// GPU task base class
// (c) EB Nov 2009

#ifndef GPUTask_h
#define GPUTask_h

#include "Config.h"

class GPUTask
{
public:

  // Create a new context, with a command queue, and the given program.
  // PROGRAMFILE may be 0 if no program is needed.
  // Check if the class is valid after construction with isValid().
  GPUTask(const char * programFile,Logger * log);

  // Destructor. Release CL objects
  virtual ~GPUTask();

  // Check if the task is well-defined.  May be redefined in derived classes
  // to add more checks (call base class if redefined).
  virtual bool isValid() { return (mContext != 0) && (mQueue != 0); }

  // Functions to redefine in derived classes

  // Run test producing SZ bytes of output.
  // Return throughput in MB/s of generated output,
  // or -1 on error.
  virtual double run(int workgroupSize,size_t sz) = 0;

protected:

  // Build the program with OPTIONS.  Error messages are put in LOG.
  // Return TRUE if OK, FALSE on error.
  bool buildProgram(const char * options,Logger * log);

  // Create a kernel for the current program, unless undefined
  cl::Kernel * createKernel(const char * name);

  inline cl::Context * getContext() { return mContext; }
  inline cl::CommandQueue * getQueue() { return mQueue; }
  inline cl::Program * getProgram() { return mProgram; }

private:

  // No copy
  GPUTask(const GPUTask &);
  GPUTask & operator = (const GPUTask &);

  cl::Context * mContext;
  cl::CommandQueue * mQueue;
  cl::Program * mProgram;
};

#endif // GPUTask_h
