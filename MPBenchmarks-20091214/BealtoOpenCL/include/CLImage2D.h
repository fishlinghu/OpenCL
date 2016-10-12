// OpenCL image2D object
// (c) EB Nov 2009

#ifndef CLImage2D_h
#define CLImage2D_h

#include <CL/cl.h>
#include "CLError.h"
#include "CLMemoryObject.h"

namespace cl {

  class Image2D : public MemoryObject
{
public:

  // Instances of this class are created from a Context.

  // Destructor
  virtual ~Image2D() { }

private:

  Image2D(); // not implemented
  Image2D(cl_mem x) : MemoryObject(x) { }

  friend class Context;
  friend class CommandQueue;
  friend class Kernel;

}; // class Image2D

} // namespace

#endif // CLImage2D_h
