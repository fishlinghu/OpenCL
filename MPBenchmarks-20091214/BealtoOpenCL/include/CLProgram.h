// OpenCL program object
// (c) EB Sep 2009

#ifndef CLProgram_h
#define CLProgram_h

#include <CL/cl.h>
#include <string>
#include <vector>
#include "CLError.h"
#include "CLKernel.h"

namespace cl {

class Program
{
public:

  // Instances of this class are created from a Context.

  // Copy constructor
  Program(const Program & a) : mX(a.mX)
  { clRetainProgram(mX); }

  // = operator
  Program & operator = (const Program & a)
  {
    if (a.mX != mX)
    {
      clReleaseProgram(mX);
      mX = a.mX;
      clRetainProgram(mX);
    }
    return *this;
  }

  // Destructor
  virtual ~Program()
  { clReleaseProgram(mX); }

  // Build the program (for all devices, and blocking)
  // See clBuildProgram for the arguments.
  // Return TRUE if OK, and FALSE otherwise.
  bool build(const char * options,std::string & buildErrors);

  // Get program binary for first device
  bool getBinary(std::vector<unsigned char> & binary);

  // Get a kernel for this program
  // Return a new instance if OK, and 0 otherwise.
  Kernel * createKernel(const char * kernel_name)
  {
    cl_int status;
    cl_kernel k = clCreateKernel(mX,kernel_name,&status);
    REPORT_OPENCL_STATUS(status);
    if (k == 0) return 0; // Failed
    return new Kernel(k);
  }

private:

  Program(); // not implemented
  Program(cl_program x) : mX(x) { }

  // OpenCL handle (always non 0)
  cl_program mX;

  friend class Context;

}; // class Buffer

} // namespace

#endif // CLProgram_h
