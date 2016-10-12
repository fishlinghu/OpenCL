// CPU task base class
// (c) EB Nov 2009

#ifndef CPUTask_h
#define CPUTask_h

class CPUTask
{
public:

  // Destructor
  virtual ~CPUTask() { }

  // Run task on NTHREADS threads and output size SZ.
  // If SZ is not a multiple of NTHREADS, SZ is rounded down to
  // the closest multiple.
  // Return throughput in MB/s of output, or -1 on error.
  virtual double run(int nThreads,size_t sz) = 0;

};

#endif // CPUTask_h
