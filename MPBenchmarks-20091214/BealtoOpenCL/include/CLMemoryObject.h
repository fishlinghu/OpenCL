// OpenCL memory object
// (c) EB Dec 2009

#ifndef CLMemoryObject_h
#define CLMemoryObject_h

#include <CL/cl.h>
#include "CLError.h"

namespace cl {

class MemoryObject
{
public:

  // Instances of this class are created from a Context.

  // Copy constructor
  MemoryObject(const MemoryObject & a) : mX(a.mX)
  { clRetainMemObject(mX); }

  // = operator
  MemoryObject & operator = (const MemoryObject & a)
  {
    if (a.mX != mX)
    {
      clReleaseMemObject(mX);
      mX = a.mX;
      clRetainMemObject(mX);
    }
    return *this;
  }

  // Destructor
  virtual ~MemoryObject()
  { clReleaseMemObject(mX); }

  // Get memory object info.
  // See clGetMemObjectInfo for the arguments.
  bool getMemObjectInfo(cl_mem_info param_name,size_t param_value_size,void * param_value,size_t * param_value_size_ret)
  {
    cl_int status = clGetMemObjectInfo(mX,param_name,param_value_size,param_value,param_value_size_ret);
    REPORT_OPENCL_STATUS(status);
    return (status == CL_SUCCESS);
  }

  // Template version suitable for scalar types
  template <class T> bool getMemObjectInfo(cl_mem_info param_name,T & x)
  { return getMemObjectInfo(param_name,sizeof(T),&x,0); }

  // Specific info entries
  size_t getSize()
  {
    size_t x;
    bool ok = getMemObjectInfo(CL_MEM_SIZE,x);
    return (ok)?x:0;
  }

private:

  MemoryObject(); // not implemented
  MemoryObject(cl_mem x) : mX(x) { }

  // OpenCL handle (always non 0)
  cl_mem mX;

  friend class Context;
  friend class CommandQueue;
  friend class Kernel;
  friend class Buffer;
  friend class Image2D;

}; // class MemoryObject

} // namespace

#endif // CLMemoryObject_h
