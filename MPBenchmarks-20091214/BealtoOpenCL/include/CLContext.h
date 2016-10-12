// OpenCL context
// (c) EB Sep 2009

#ifndef CLContext_h
#define CLContext_h

#include <CL/cl.h>
#include <vector>
#include <string>

namespace cl {

class Buffer;
class Image2D;
class CommandQueue;
class Program;

class Context
{
public:

  // Copy constructor
  Context(const Context & a);

  // = operator
  Context & operator = (const Context & a);

  // Destructor
  virtual ~Context();

  // Create default context (including all devices with the given type).
  // Return a new instance if OK, and 0 otherwise.
  static Context * create(cl_device_type deviceType = CL_DEVICE_TYPE_GPU);

  // Get number of associated devices
  int getNDevices() const;

  // Create a new command queue object.
  // D is the index of the device ine the context (0..NDevices-1)
  // See clCreateCommandQueue for the arguments.
  // Return a new instance if OK, and 0 otherwise.
  CommandQueue * createCommandQueue(int d,cl_command_queue_properties properties);

  // Create a new buffer object.
  // See clCreateBuffer for the arguments.
  // Return a new instance if OK, and 0 otherwise.
  Buffer * createBuffer(cl_mem_flags flags,size_t size,void * host_ptr = 0);

  // Create a new image2D object.
  // See clCreateImage2D for the arguments.
  // Return a new instance if OK, and 0 otherwise.
  Image2D * createImage2D(cl_mem_flags flags,const cl_image_format * image_format,
    size_t width,size_t height,size_t pitch = 0,void * host_ptr = 0);

  // Create a R / RG / RGBA image2D object.
  // channel_type is passed in a cl_image_format structure.
  Image2D * createRImage2D(cl_mem_flags flags,cl_channel_type data_type,
    size_t width,size_t height,size_t pitch = 0,void * host_ptr = 0)
  {
    cl_image_format f;
    f.image_channel_order = CL_R;
    f.image_channel_data_type = data_type;
    return createImage2D(flags,&f,width,height,pitch,host_ptr);
  }
  Image2D * createRGImage2D(cl_mem_flags flags,cl_channel_type data_type,
    size_t width,size_t height,size_t pitch = 0,void * host_ptr = 0)
  {
    cl_image_format f;
    f.image_channel_order = CL_RG;
    f.image_channel_data_type = data_type;
    return createImage2D(flags,&f,width,height,pitch,host_ptr);
  }
  Image2D * createRGBAImage2D(cl_mem_flags flags,cl_channel_type data_type,
    size_t width,size_t height,size_t pitch = 0,void * host_ptr = 0)
  {
    cl_image_format f;
    f.image_channel_order = CL_RGBA;
    f.image_channel_data_type = data_type;
    return createImage2D(flags,&f,width,height,pitch,host_ptr);
  }

  // Create a new program from source strings.
  // See clCreateProgramWithSource for the arguments.
  // Return a new instance if OK, and 0 otherwise.
  Program * createProgramWithSource(cl_uint count,const char ** strings,const size_t * lengths = 0);

  // Create a new program from source files.
  // See clCreateProgramWithSource for the arguments.
  // Return a new instance if OK, and 0 otherwise.
  Program * createProgramWithFiles(cl_uint count,const char ** filenames);
  Program * createProgramWithFile(const char * filename)
  { return createProgramWithFiles(1,&filename); }
  Program * createProgramWithFiles(const std::vector<std::string> & filenames);

  // Get device info for device D (0..NDevices-1).
  // See clGetDeviceInfo for the arguments.
  bool getDeviceInfo(int d,cl_device_info param_name,size_t param_value_size,void * param_value,size_t * param_value_size_ret);

  // Template version suitable for scalar types
  template <class T> bool getDeviceInfo(int d,cl_device_info param_name,T & x)
  { return getDeviceInfo(d,param_name,sizeof(T),&x,0); }

  // Get all info for device D.
  // INFO receives the result (one attribute per line), cleared by the call.
  bool getAllDeviceInfo(int d,std::string & info);

  // Specific info entries.
  // D is the device index in context, 0..NDevices-1.
  size_t getDeviceMaxMemAllocSize(int d = 0)
  {
    cl_ulong x;
    if (!getDeviceInfo(d,CL_DEVICE_MAX_MEM_ALLOC_SIZE,x)) return 0; // Failed
    return (size_t)x;
  }
  bool getDeviceImageSupport(int d = 0)
  {
    cl_bool x;
    if (!getDeviceInfo(d,CL_DEVICE_IMAGE_SUPPORT,x)) return false; // Failed
    return (x)?true:false;
  }
  size_t getDeviceImage2DMaxWidth(int d = 0)
  {
    size_t x;
    if (!getDeviceInfo(d,CL_DEVICE_IMAGE2D_MAX_WIDTH,x)) return 0; // Failed
    return x;
  }
  size_t getDeviceImage2DMaxHeight(int d = 0)
  {
    size_t x;
    if (!getDeviceInfo(d,CL_DEVICE_IMAGE2D_MAX_HEIGHT,x)) return 0; // Failed
    return x;
  }

private:

  // This constructor is private, use create() to instanciate this class
  Context();

  // OpenCL handle (always non 0)
  cl_context mX;

  // Devices associated with this context
  std::vector<cl_device_id> mDevices;

}; // class Context

} // namespace

#endif // CLContext_h
