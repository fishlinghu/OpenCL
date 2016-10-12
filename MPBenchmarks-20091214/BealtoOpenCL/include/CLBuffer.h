// OpenCL buffer object
// (c) EB Sep 2009

#ifndef CLBuffer_h
#define CLBuffer_h

#include <CL/cl.h>
#include "CLMemoryObject.h"
#include "CLError.h"

namespace cl {

class Buffer : public MemoryObject
{
public:

  // Instances of this class are created from a Context.

  // Destructor
  virtual ~Buffer() { }

private:

  Buffer(); // not implemented
  Buffer(cl_mem x) : MemoryObject(x) { }

  friend class Context;
  friend class CommandQueue;
  friend class Kernel;
};

} // namespace

#endif // CLBuffer_h
