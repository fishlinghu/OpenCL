// OpenCL command queue object
// (c) EB Sep 2009

#ifndef CLCommandQueue_h
#define CLCommandQueue_h

#include <CL/cl.h>
#include "CLBuffer.h"
#include "CLEvent.h"
#include "CLEventList.h"
#include "CLKernel.h"

namespace cl {

class CommandQueue
{
public:

  // Instances of this class are created from a Context.

  // Copy constructor
  CommandQueue(const CommandQueue & a) : mX(a.mX)
  { clRetainCommandQueue(mX); }

  // = operator
  CommandQueue & operator = (const CommandQueue & a)
  {
    if (a.mX != mX)
    {
      clReleaseCommandQueue(mX);
      mX = a.mX;
      clRetainCommandQueue(mX);
    }
    return *this;
  }

  // Destructor
  virtual ~CommandQueue()
  { clReleaseCommandQueue(mX); }

  // Commands
  // All commands return an Event instance.
  // If the call fails, the returned event is invalid.

  // Read buffer to host memory
  Event readBuffer(Buffer * b,cl_bool blocking_read,size_t offset,size_t cb,void * ptr,const EventList & wait_list = EventList())
  {
    if (b == 0) return Event(0); // Invalid
    cl_uint num_events = 0;
    const cl_event * events = 0;
    wait_list.getParams(num_events,events);
    cl_event e = 0;
    cl_int status = clEnqueueReadBuffer(mX,b->mX,blocking_read,offset,cb,ptr,num_events,events,&e);
    REPORT_OPENCL_STATUS(status);
    if (status != CL_SUCCESS) e = 0;
    return Event(e);
  }

  // Write buffer from host memory
  Event writeBuffer(Buffer * b,cl_bool blocking_write,size_t offset,size_t cb,const void * ptr,const EventList & wait_list = EventList())
  {
    if (b == 0) return Event(0); // Invalid
    cl_uint num_events = 0;
    const cl_event * events = 0;
    wait_list.getParams(num_events,events);
    cl_event e = 0;
    cl_int status = clEnqueueWriteBuffer(mX,b->mX,blocking_write,offset,cb,ptr,num_events,events,&e);
    REPORT_OPENCL_STATUS(status);
    if (status != CL_SUCCESS) e = 0;
    return Event(e);
  }

  // Copy buffers
  Event copyBuffer(Buffer * src,Buffer * dst,size_t src_offset,size_t dst_offset,size_t cb,const EventList & wait_list = EventList())
  {
    if (src == 0 || dst == 0) return Event(0); // Invalid
    cl_uint num_events = 0;
    const cl_event * events = 0;
    wait_list.getParams(num_events,events);
    cl_event e = 0;
    cl_int status = clEnqueueCopyBuffer(mX,src->mX,dst->mX,src_offset,dst_offset,cb,num_events,events,&e);
    REPORT_OPENCL_STATUS(status);
    if (status != CL_SUCCESS) e = 0;
    return Event(e);
  }

  // Map buffer.  The mapped address is put in ADDRESS.
  template <typename T> Event mapBuffer(Buffer * b,T * & address,cl_bool blocking_map,cl_map_flags map_flags,size_t offset,size_t cb,const EventList & wait_list = EventList())
  {
    if (b == 0) return Event(0);
    cl_uint num_events = 0;
    const cl_event * events = 0;
    wait_list.getParams(num_events,events);
    cl_event e = 0;
    cl_int status;
    address = (T *)clEnqueueMapBuffer(mX,b->mX,blocking_map,map_flags,offset,cb,num_events,events,&e,&status);
    REPORT_OPENCL_STATUS(status);
    if (status != CL_SUCCESS) e = 0;
    return Event(e);
  }

  // Map full buffer.  Queries its size, and maps all its contents.  The mapped address is put in ADDRESS.
  template <typename T> Event mapBuffer(Buffer * b,T * & address,cl_bool blocking_map,cl_map_flags map_flags,const EventList & wait_list = EventList())
  {
    if (b == 0) return Event(0); // Invalid buffer
    size_t sz = b->getSize();
    if (sz == 0) return Event(0); // Invalid size
    return mapBuffer<T>(b,address,blocking_map,map_flags,0,sz,wait_list);
  }

  // Unmap memory object. (Buffer, Image2D, etc.)
  Event unmapMemoryObject(MemoryObject * m,void * address,const EventList & wait_list = EventList())
  {
    if (m == 0) return Event(0);
    cl_uint num_events = 0;
    const cl_event * events = 0;
    wait_list.getParams(num_events,events);
    cl_event e = 0;
    cl_int status = clEnqueueUnmapMemObject(mX,m->mX,address,num_events,events,&e);
    REPORT_OPENCL_STATUS(status);
    if (status != CL_SUCCESS) e = 0;
    return Event(e);
  }

  // Enqueue 1D kernel execution
  // K is the kernel to run.
  // N is the total number of work items (global work size).
  // G is the number of work items inside a work group. G must divide N.
  // G can be 0, in which case the OpenCL implementation will choose the best value.
  Event execKernel1(Kernel * k,size_t n,size_t g,const EventList & wait_list = EventList())
  {
    if (k == 0) return Event(0); // Invalid
    cl_uint num_events = 0;
    const cl_event * events = 0;
    wait_list.getParams(num_events,events);
    cl_event e = 0;
    size_t * pgw = &n;
    size_t * plw = (g>0)?(&g):0;
    cl_int status = clEnqueueNDRangeKernel(mX,k->mX,1,0,pgw,plw,num_events,events,&e);
    REPORT_OPENCL_STATUS(status);
    if (status != CL_SUCCESS) e = 0;
    return Event(e);
  }

  // Enqueue 2D kernel execution
  // K is the kernel to run.
  // NX*NY is the total number of work items (global work size).
  // GX*GY is the number of work items inside a work group. G<d> must divide N<d>.
  Event execKernel2(Kernel * k,size_t nx,size_t ny,size_t gx,size_t gy,const EventList & wait_list = EventList())
  {
    if (k == 0) return Event(0); // Invalid
    cl_uint num_events = 0;
    const cl_event * events = 0;
    wait_list.getParams(num_events,events);
    cl_event e = 0;
    size_t pgw[2]; pgw[0] = nx; pgw[1] = ny;
    size_t plw[2]; plw[0] = gx; plw[1] = gy;
    cl_int status = clEnqueueNDRangeKernel(mX,k->mX,2,0,pgw,plw,num_events,events,&e);
    REPORT_OPENCL_STATUS(status);
    if (status != CL_SUCCESS) e = 0;
    return Event(e);
  }

  //
  // Execution control
  //

  // Enqueue a marker
  Event mark()
  {
    cl_event e = 0;
    cl_int status = clEnqueueMarker(mX,&e);
    REPORT_OPENCL_STATUS(status);
    if (status != CL_SUCCESS) e = 0;
    return Event(e);
  }

  // Insert a wait point for a specific list of events
  bool wait(const EventList & wait_list)
  {
    cl_uint num_events = 0;
    const cl_event * events = 0;
    wait_list.getParams(num_events,events);
    if (num_events == 0) return true; // Nop
    cl_int status = clEnqueueWaitForEvents(mX,num_events,events);
    REPORT_OPENCL_STATUS(status);
    return (status == CL_SUCCESS);
  }

  // Insert a wait point for all events. (barrier)
  bool wait()
  {
    cl_int status = clEnqueueBarrier(mX);
    REPORT_OPENCL_STATUS(status);
    return (status == CL_SUCCESS);
  }

  // Blocks until all queued commands have been submitted to the device.
  bool flush()
  {
    cl_int status = clFlush(mX);
    REPORT_OPENCL_STATUS(status);
    return (status == CL_SUCCESS);
  }

  // Blocks until all queued commands have been executed.
  bool finish()
  {
    cl_int status = clFinish(mX);
    REPORT_OPENCL_STATUS(status);
    return (status == CL_SUCCESS);
  }

private:

  // Private constructors
  CommandQueue(); // not implemented
  CommandQueue(cl_command_queue x) : mX(x) { }

  // OpenCL handle (always non 0)
  cl_command_queue mX;

  friend class Context;

}; // class CommandQueue

} // namespace

#endif // CLCommandQueue_h
