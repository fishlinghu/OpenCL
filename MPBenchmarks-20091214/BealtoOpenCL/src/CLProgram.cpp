// OpenCL program object
// (c) EB Sep 2009

#include <stdlib.h>
#include <string.h>
#include "CLProgram.h"

using namespace cl;

bool Program::build(const char * options,std::string & buildErrors)
{
  buildErrors.clear();
  cl_int status;

  // Get devices
  cl_uint nDevices = 0;
  status = clGetProgramInfo(mX,CL_PROGRAM_NUM_DEVICES,sizeof(nDevices),&nDevices,0);
  REPORT_OPENCL_STATUS(status);
  if (status != CL_SUCCESS || nDevices == 0) { buildErrors.append("clGetProgramInfo NUM_DEVICES failed\n"); return false; }
  std::vector<cl_device_id> devices;
  devices.resize(nDevices,0);
  status = clGetProgramInfo(mX,CL_PROGRAM_DEVICES,sizeof(cl_device_id)*nDevices,&(devices[0]),0);
  REPORT_OPENCL_STATUS(status);
  if (status != CL_SUCCESS) { buildErrors.append("clGetProgramInfo DEVICES failed\n"); return false; }

  // Build program
  status = clBuildProgram(mX,nDevices,&(devices[0]),options,0,0);
  REPORT_OPENCL_STATUS(status);
  if (status == CL_SUCCESS) return true; // OK

  // Get build info for each device
  for (unsigned int i=0;i<nDevices;i++)
  {
    cl_device_id d = devices[i];
    char aux[4000];
    status = clGetProgramBuildInfo(mX,d,CL_PROGRAM_BUILD_LOG,4000,aux,0);
    REPORT_OPENCL_STATUS(status);
    if (status != CL_SUCCESS) { buildErrors.append("clGetProgramBuildInfo failed\n"); continue; }
    buildErrors.append(aux);
  }

  buildErrors.append("Build failed\n");
  return false;
}

// Get program binary for first device
bool Program::getBinary(std::vector<unsigned char> & binary)
{
  cl_int status;
  binary.clear();

  // Get devices
  cl_uint nDevices = 0;
  status = clGetProgramInfo(mX,CL_PROGRAM_NUM_DEVICES,sizeof(nDevices),&nDevices,0);
  REPORT_OPENCL_STATUS(status);
  if (status != CL_SUCCESS || nDevices == 0) return false;
  // Get binary sizes for all devices
  std::vector<size_t> binarySize(nDevices,0);
  status = clGetProgramInfo(mX,CL_PROGRAM_BINARY_SIZES,nDevices*sizeof(size_t),&(binarySize[0]),0);
  REPORT_OPENCL_STATUS(status);
  if (status != CL_SUCCESS) return false;
  // Alloc buffers
  std::vector<unsigned char *> buf(nDevices,0);
  for (cl_uint i=0;i<nDevices;i++)
  {
    if (binarySize[i]>0) buf[i] = (unsigned char *)malloc(binarySize[i]);
  }
  status = clGetProgramInfo(mX,CL_PROGRAM_BINARIES,nDevices*sizeof(unsigned char *),&(buf[0]),0);
  REPORT_OPENCL_STATUS(status);
  if (status != CL_SUCCESS) return false;
  // Copy result for first device
  if (binarySize[0]>0)
  {
    binary.resize(binarySize[0],0);
    memcpy(&(binary[0]),buf[0],binarySize[0]);
  }
  // Free buffers
  for (cl_uint i=0;i<nDevices;i++)
  {
    if (buf[i] != 0) free(buf[i]);
  }
  return true; // OK
}

