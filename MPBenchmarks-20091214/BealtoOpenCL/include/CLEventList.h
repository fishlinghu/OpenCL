// OpenCL event list
// (c) EB Sep 2009

#ifndef CLEventList_h
#define CLEventList_h

#include <CL/cl.h>
#include <vector>
#include "CLEvent.h"

namespace cl {

class EventList
{
public:

  // Constructor from a list of events
  EventList() { }
  EventList(const Event & e1) { insert(e1); }
  EventList(const Event & e1,const Event & e2) { insert(e1); insert(e2); }
  EventList(const Event & e1,const Event & e2,const Event & e3) { insert(e1); insert(e2); insert(e3); }

  // Copy constructor
  EventList(const EventList & e)
  {
    for (event_list_t::const_iterator it = e.mX.begin(); it != e.mX.end(); it++) insert(*it);
  }

  // = operator
  EventList & operator = (const EventList & e)
  {
    if (&e == this) return *this;
    clear();
    for (event_list_t::const_iterator it = e.mX.begin(); it != e.mX.end(); it++) insert(*it);
    return *this;
  }

  // Destructor
  ~EventList() { clear(); }

  // Clear the list
  void clear()
  {
    for (event_list_t::iterator it = mX.begin(); it != mX.end(); it++) clReleaseEvent(*it);
    mX.clear();
  }

  // Insert an event to the list.  Ignore if E is invalid.
  void insert(const Event & e)
  {
    cl_event x = e.mX;
    if (x == 0) return;
    clRetainEvent(x);
    mX.push_back(x);
  }

  // Get the number of events in the list
  int size() const { return (int)mX.size(); }

  // Is the list empty?
  bool empty() const { return mX.empty(); }

  // Wait for all the events to complete.
  // Return TRUE if OK, and FALSE otherwise.
  // If the list is empty, return TRUE.
  bool wait()
  {
    if (mX.empty()) return true; // OK, empty
    cl_uint num_events;
    const cl_event * events;
    getParams(num_events,events);
    cl_int status = clWaitForEvents(num_events,events);
    REPORT_OPENCL_STATUS(status);
    return (status == CL_SUCCESS);
  }

  // Get CL parameters for the calls
  void getParams(cl_uint & num_events,const cl_event * & events) const
  {
    num_events = (cl_uint)mX.size();
    if (num_events == 0) events = 0;
    else events = &(mX[0]);
  }

private:

  typedef std::vector<cl_event> event_list_t;

  // OpenCL handles
  event_list_t mX;

}; // class EventList

} // namespace

#endif // CLEventList_h
