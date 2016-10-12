// OpenCL event object
// (c) EB Sep 2009

#ifndef CLEvent_h
#define CLEvent_h

#include <CL/cl.h>
#include "CLError.h"

namespace cl {

class Event
{
public:

  // Instances of this class are created by calls in a CommandQueue.

  // Copy constructor
  Event(const Event & a) : mX(a.mX)
  { if (mX != 0) clRetainEvent(mX); }

  // = operator
  Event & operator = (const Event & a)
  {
    if (a.mX != mX)
    {
      if (mX != 0) clReleaseEvent(mX);
      mX = a.mX;
      if (mX != 0) clRetainEvent(mX);
    }
    return *this;
  }

  // Destructor
  virtual ~Event()
  { if (mX != 0) clReleaseEvent(mX); }

  // Check if the event is valid
  bool isValid() const { return (mX != 0); }

  // Wait for this event to complete.
  // Return TRUE if OK, and FALSE otherwise.
  // If the event is invalid, return TRUE.
  bool wait()
  {
    if (mX == 0) return true; // OK, event is invalid
    cl_int status = clWaitForEvents(1,&mX);
    REPORT_OPENCL_STATUS(status);
    return (status == CL_SUCCESS);
  }

private:

  Event(); // not implemented
  Event(cl_event x) : mX(x) { }

  // OpenCL handle (MAY BE 0)
  cl_event mX;

  friend class CommandQueue;
  friend class EventList;

}; // class Event

} // namespace

#endif // CLEvent_h
