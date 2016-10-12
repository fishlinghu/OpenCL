// OpenCL kernel object
// (c) EB Sep 2009

#ifndef CLKernel_h
#define CLKernel_h

#include <CL/cl.h>
#include "CLError.h"
#include "CLBuffer.h"

namespace cl {

class Kernel
{
public:

  // Instances of this class are created from a Program.

  // Copy constructor
  Kernel(const Kernel & a) : mX(a.mX)
  { clRetainKernel(mX); }

  // = operator
  Kernel & operator = (const Kernel & a)
  {
    if (a.mX != mX)
    {
      clReleaseKernel(mX);
      mX = a.mX;
      clRetainKernel(mX);
    }
    return *this;
  }

  // Destructor
  virtual ~Kernel()
  { clReleaseKernel(mX); }

  // Set kernel argument
  // See clSetKernelArg for arguments.
  // Return TRUE if OK, and FALSE otherwise.
  bool setArg(cl_uint arg_index,size_t arg_size,const void * arg_value)
  {
    cl_int status = clSetKernelArg(mX,arg_index,arg_size,arg_value);
    REPORT_OPENCL_STATUS(status);
    return (status == CL_SUCCESS);
  }
  // Set kernel argument (Buffer)
  // Return TRUE if OK, and FALSE otherwise.
  bool setArg(cl_uint arg_index,Buffer * b)
  {
    if (b == 0) return false; // Invalid
    cl_mem m = b->mX;
    return setArg(arg_index,sizeof(m),(const void *)&m);
  }
  // Set kernel argument (other types)
  // Return TRUE if OK, and FALSE otherwise.
  bool setArg(cl_int arg_index,cl_int x)
  { return setArg(arg_index,sizeof(x),(const void *)&x); }
  bool setArg(cl_int arg_index,cl_float x)
  { return setArg(arg_index,sizeof(x),(const void *)&x); }

private:

  Kernel(); // not implemented
  Kernel(cl_kernel x) : mX(x) { }

  // OpenCL handle (always non 0)
  cl_kernel mX;

  friend class Program;
  friend class CommandQueue;

}; // class Kernel

} // namespace

#endif // CLKernel_h
