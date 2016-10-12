// OpenCL context
// (c) EB Sep 2009

#define _CRT_SECURE_NO_WARNINGS
#include <sys/stat.h>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include "CLError.h"
#include "CLContext.h"
#include "CLCommandQueue.h"
#include "CLBuffer.h"
#include "CLImage2D.h"
#include "CLProgram.h"

#ifdef Linux
#define _stat stat
#endif

using namespace cl;

Context::Context() : mX(0)
{
}

Context::Context(const Context & a) : mX(a.mX)
{
  clRetainContext(mX);
}

Context & Context::operator = (const Context & a)
{
  if (a.mX != mX)
  {
    clReleaseContext(mX);
    mX = a.mX;
    clRetainContext(mX);
  }
  return *this;
}

Context::~Context()
{
  clReleaseContext(mX);
}

Context * Context::create(cl_device_type deviceType)
{
  cl_platform_id bestPlatform = 0;
  std::vector<cl_device_id> devices;
  int status;

  // Platforms
  cl_uint nPlatforms = 0;
  std::vector<cl_platform_id> pIDs;
  status = clGetPlatformIDs(0,0,&nPlatforms);
  REPORT_OPENCL_STATUS(status);
  if (status != CL_SUCCESS || nPlatforms == 0) return 0; // Failed
  pIDs.resize(nPlatforms,0);
  status = clGetPlatformIDs(nPlatforms,&(pIDs[0]),&nPlatforms);
  REPORT_OPENCL_STATUS(status);
  if (status != CL_SUCCESS) return 0; // Failed

  // printf("NPlatforms: %u\n",nPlatforms);

  // Devices for each platform
  for (unsigned int i=0;i<nPlatforms;i++)
  {
    cl_platform_id p = pIDs[i];

    // Get all GPU devices for this platform
    cl_uint nDevices = 0;
    status = clGetDeviceIDs(p,deviceType,0,0,&nDevices);
    // printf("NDevices: %u\n",nDevices);
    REPORT_OPENCL_STATUS(status);
    if (status != CL_SUCCESS || nDevices == 0) continue;
    devices.resize(nDevices,0);
    status = clGetDeviceIDs(p,deviceType,nDevices,&(devices[0]),&nDevices);
    REPORT_OPENCL_STATUS(status);
    if (status != CL_SUCCESS) { devices.clear(); continue; }

    bestPlatform = p;
    break; // OK
  }
  if (devices.empty())
    {
      printf("No device found\n");
      return 0;
    }

  cl_context_properties cp[4];
  cp[0] = CL_CONTEXT_PLATFORM;
  cp[1] = (cl_context_properties)bestPlatform;
  cp[2] = cp[3] = 0;
  cl_context c = clCreateContext(cp,(int)devices.size(),&(devices[0]),0,0,0);
  if (c == 0) return 0;

  Context * result = new Context();
  result->mX = c;
  result->mDevices = devices;

  return result;
}

int Context::getNDevices() const
{
  return (int)mDevices.size();
}

CommandQueue * Context::createCommandQueue(int d,cl_command_queue_properties properties)
{
  cl_int status;
  if (d<0 || d>=(int)mDevices.size()) return 0; // Invalid D
  cl_command_queue q = clCreateCommandQueue(mX,mDevices[d],properties,&status);
  REPORT_OPENCL_STATUS(status);
  if (q == 0) return 0; // Failed
  return new CommandQueue(q);
}

Buffer * Context::createBuffer(cl_mem_flags flags,size_t size,void * host_ptr)
{
  cl_int status;
  cl_mem m = clCreateBuffer(mX,flags,size,host_ptr,&status);
  REPORT_OPENCL_STATUS(status);
  if (m == 0) return 0; // Failed
  return new Buffer(m);
}

Image2D * Context::createImage2D(cl_mem_flags flags,const cl_image_format * image_format,
    size_t width,size_t height,size_t pitch,void * host_ptr)
{
  cl_int status;
  cl_mem m = clCreateImage2D(mX,flags,image_format,width,height,pitch,host_ptr,&status);
  REPORT_OPENCL_STATUS(status);
  if (m == 0) return 0; // Failed
  return new Image2D(m);
}

Program * Context::createProgramWithSource(cl_uint count,const char ** strings,const size_t * lengths)
{
  cl_int status;
  cl_program p = clCreateProgramWithSource(mX,count,strings,lengths,&status);
  REPORT_OPENCL_STATUS(status);
  if (p == 0) return 0; // Failed
  return new Program(p);
}

Program * Context::createProgramWithFiles(cl_uint count,const char ** filenames)
{
  if (count == 0 || filenames == 0) return 0; // Invalid

  std::vector<char *> strings;
  std::vector<size_t> lengths;
  strings.resize(count,0);
  lengths.resize(count,0);
  bool ok = true;
  for (unsigned int i=0;i<count;i++)
  {
    // Get file size and check it exists
    struct _stat s;
    int status = ::_stat(filenames[i],&s);
    if (status != 0) { ok = false; break; }
    size_t sz = s.st_size;
    if (sz == 0) { ok = false; break; }

    // Read the file
    FILE * f = fopen(filenames[i],"rb");
    if (f == 0) { ok = false; break; }
    char * buffer = (char *)malloc(sz);
    strings[i] = buffer;
    lengths[i] = sz;

    size_t readBlocks = fread(buffer,sz,1,f);
    fclose(f);
    if (readBlocks != 1) { ok = false; break; } // read failed
  }
  Program * result = 0;
  if (ok) result = createProgramWithSource(count,(const char **)&(strings[0]),&(lengths[0]));

  // Cleanup
  for (unsigned int i=0;i<count;i++) if (strings[i] != 0)
  { free(strings[i]); }

  return result;
}

Program * Context::createProgramWithFiles(const std::vector<std::string> & filenames)
{
  cl_uint n = (int)filenames.size();
  if (n == 0) return 0; // Empty
  std::vector<const char *> files(n,0);
  for (cl_uint i=0;i<n;i++)
  {
    if (filenames[i].empty()) continue; // 0 if empty
    files[i] = filenames[i].c_str();
  }
  return createProgramWithFiles(n,&(files[0]));
}

bool Context::getDeviceInfo(int d,cl_device_info param_name,size_t param_value_size,void * param_value,size_t * param_value_size_ret)
{
  if (d<0 || d>=(int)mDevices.size()) return 0; // Invalid D
  cl_int status = clGetDeviceInfo(mDevices[d],param_name,param_value_size,param_value,param_value_size_ret);
  REPORT_OPENCL_STATUS(status);
  return (status == CL_SUCCESS);
}

bool Context::getAllDeviceInfo(int d,std::string & info)
{
  cl_uint ui;
  cl_ulong ul;
  cl_bool b;
  info.clear();
  std::ostringstream o;
  o << "Device info (ID=0x" << mDevices[d] << ")\n";

  cl_device_type dType;
  if (!getDeviceInfo(d,CL_DEVICE_TYPE,dType)) return false;
  o << "- type:";
  if (dType & CL_DEVICE_TYPE_CPU) o << " cpu";
  if (dType & CL_DEVICE_TYPE_GPU) o << " gpu";
  if (dType & CL_DEVICE_TYPE_ACCELERATOR) o << " accelerator";
  o << "\n";

  if (!getDeviceInfo(d,CL_DEVICE_VENDOR_ID,ui)) return false;
  o << "- vendor id: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MAX_COMPUTE_UNITS,ui)) return false;
  o << "- max compute units: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,ui)) return false;
  o << "- max work item dimensions: " << ui << "\n";
  size_t sz[24];
  if (!getDeviceInfo(d,CL_DEVICE_MAX_WORK_ITEM_SIZES,24*sizeof(size_t),sz,0)) return false;
  o << "- max work item sizes:";
  for (cl_uint i=0;i<ui;i++) o << " " << sz[i];
  o << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MAX_WORK_GROUP_SIZE,sz[0])) return false;
  o << "- max work group size: " << sz[0] << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,ui)) return false;
  o << "- preferred vector width char: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,ui)) return false;
  o << "- preferred vector width short: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,ui)) return false;
  o << "- preferred vector width int: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,ui)) return false;
  o << "- preferred vector width long: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,ui)) return false;
  o << "- preferred vector width float: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,ui)) return false;
  o << "- preferred vector width double: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MAX_CLOCK_FREQUENCY,ui)) return false;
  o << "- max clock frequency: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_ADDRESS_BITS,ui)) return false;
  o << "- address bits: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MAX_MEM_ALLOC_SIZE,ul)) return false;
  o << "- max mem alloc size: " << ul << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_IMAGE_SUPPORT,b)) return false;
  o << "- image support: " << (b?"yes":"no") << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MAX_READ_IMAGE_ARGS,ui)) return false;
  o << "- max read image args: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MAX_WRITE_IMAGE_ARGS,ui)) return false;
  o << "- max write image args: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_IMAGE2D_MAX_WIDTH,sz[0])) return false;
  o << "- image2d max width: " << sz[0] << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_IMAGE2D_MAX_HEIGHT,sz[0])) return false;
  o << "- image2d max height: " << sz[0] << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_IMAGE3D_MAX_WIDTH,sz[0])) return false;
  o << "- image3d max width: " << sz[0] << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_IMAGE3D_MAX_HEIGHT,sz[0])) return false;
  o << "- image3d max height: " << sz[0] << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_IMAGE3D_MAX_DEPTH,sz[0])) return false;
  o << "- image3d max depth: " << sz[0] << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MAX_SAMPLERS,ui)) return false;
  o << "- max samplers: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MAX_PARAMETER_SIZE,sz[0])) return false;
  o << "- max parameter size: " << sz[0] << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MEM_BASE_ADDR_ALIGN,ui)) return false;
  o << "- mem base addr align: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE,ui)) return false;
  o << "- min data type align size: " << ui << "\n";

  cl_device_fp_config fpc;
  if (!getDeviceInfo(d,CL_DEVICE_SINGLE_FP_CONFIG,fpc)) return false;
  o << "- single fp config:";
  if (fpc & CL_FP_DENORM) o << " denorm";
  if (fpc & CL_FP_INF_NAN) o << " inf_nan";
  if (fpc & CL_FP_ROUND_TO_NEAREST) o << " round_to_nearest";
  if (fpc & CL_FP_ROUND_TO_ZERO) o << " round_to_zero";
  if (fpc & CL_FP_ROUND_TO_INF) o << " round_to_inf";
  if (fpc & CL_FP_FMA) o << " fma";
  o << "\n";

  cl_device_mem_cache_type mct;
  if (!getDeviceInfo(d,CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,mct)) return false;
  o << "- global mem cache type:";
  if (mct == CL_NONE) o << " none";
  else if (mct == CL_READ_ONLY_CACHE) o << " read_only_cache";
  else if (mct == CL_READ_WRITE_CACHE) o << " read_write_cache";
  o << "\n";

  if (!getDeviceInfo(d,CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,ui)) return false;
  o << "- global mem cacheline size: " << ui << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,ul)) return false;
  o << "- global mem cache size: " << ul << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_GLOBAL_MEM_SIZE,ul)) return false;
  o << "- global mem size: " << ul << "\n";

  if (!getDeviceInfo(d,CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,ul)) return false;
  o << "- max constant buffer size: " << ul << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_MAX_CONSTANT_ARGS,ui)) return false;
  o << "- max constant args: " << ui << "\n";

  cl_device_local_mem_type lmt;
  if (!getDeviceInfo(d,CL_DEVICE_LOCAL_MEM_TYPE,lmt)) return false;
  o << "- local mem type:";
  if (lmt == CL_LOCAL) o << " local";
  else o << " global";
  o << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_LOCAL_MEM_SIZE,ul)) return false;
  o << "- local mem size: " << ul << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_ERROR_CORRECTION_SUPPORT,b)) return false;
  o << "- error correction support: " << (b?"yes":"no") << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_PROFILING_TIMER_RESOLUTION,sz[0])) return false;
  o << "- profiling timer resolution: " << sz[0] << " ns\n";
  if (!getDeviceInfo(d,CL_DEVICE_ENDIAN_LITTLE,b)) return false;
  o << "- endian little: " << (b?"yes":"no") << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_AVAILABLE,b)) return false;
  o << "- available: " << (b?"yes":"no") << "\n";
  if (!getDeviceInfo(d,CL_DEVICE_COMPILER_AVAILABLE,b)) return false;
  o << "- compiler available: " << (b?"yes":"no") << "\n";

  cl_device_exec_capabilities ec;
  if (!getDeviceInfo(d,CL_DEVICE_EXECUTION_CAPABILITIES,ec)) return false;
  o << "- execution capabilities:";
  if (ec & CL_EXEC_KERNEL) o << " exec_kernel";
  if (ec & CL_EXEC_NATIVE_KERNEL) o << " exec_native_kernel";
  o << "\n";

  cl_command_queue_properties qp;
  if (!getDeviceInfo(d,CL_DEVICE_QUEUE_PROPERTIES,qp)) return false;
  o << "- queue properties:";
  if (ec & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) o << " out_of_order_exec_mode";
  if (ec & CL_QUEUE_PROFILING_ENABLE) o << " profiling";
  o << "\n";

  char exts[8192];
  if (!getDeviceInfo(d,CL_DEVICE_EXTENSIONS,8192,exts,0)) return false;
  o << "- extensions: " << exts << "\n";

  info.assign(o.str());
  return true;
}
