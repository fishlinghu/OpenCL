// OpenCL error reporting
// (c) EB Sep 2009

#if CONFIG_USE_QT
#include <QtCore/QDebug>
#include <QtCore/QString>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <CL/cl.h>
#include "CLError.h"

const int NErrorCodes = 63;
static const char * ErrorCodes[NErrorCodes] = {
"CL_SUCCESS", // 0
"CL_DEVICE_NOT_FOUND", // -1
"CL_DEVICE_NOT_AVAILABLE", // -2
"CL_COMPILER_NOT_AVAILABLE", // -3
"CL_MEM_OBJECT_ALLOCATION_FAILURE", // -4
"CL_OUT_OF_RESOURCES", // -5
"CL_OUT_OF_HOST_MEMORY", // -6
"CL_PROFILING_INFO_NOT_AVAILABLE", // -7
"CL_MEM_COPY_OVERLAP", // -8
"CL_IMAGE_FORMAT_MISMATCH", // -9
"CL_IMAGE_FORMAT_NOT_SUPPORTED", // -10
"CL_BUILD_PROGRAM_FAILURE", // -11
"CL_MAP_FAILURE", // -12
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // -13..-29
"CL_INVALID_VALUE", // -30
"CL_INVALID_DEVICE_TYPE", // -31
"CL_INVALID_PLATFORM", // -32
"CL_INVALID_DEVICE", // -33
"CL_INVALID_CONTEXT", // -34
"CL_INVALID_QUEUE_PROPERTIES", // -35
"CL_INVALID_COMMAND_QUEUE", // -36
"CL_INVALID_HOST_PTR", // -37
"CL_INVALID_MEM_OBJECT", // -38
"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR", // -39
"CL_INVALID_IMAGE_SIZE", // -40
"CL_INVALID_SAMPLER", // -41
"CL_INVALID_BINARY", // -42
"CL_INVALID_BUILD_OPTIONS", // -43
"CL_INVALID_PROGRAM", // -44
"CL_INVALID_PROGRAM_EXECUTABLE", // -45
"CL_INVALID_KERNEL_NAME", // -46
"CL_INVALID_KERNEL_DEFINITION", // -47
"CL_INVALID_KERNEL", // -48
"CL_INVALID_ARG_INDEX", // -49
"CL_INVALID_ARG_VALUE", // -50
"CL_INVALID_ARG_SIZE", // -51
"CL_INVALID_KERNEL_ARGS", // -52
"CL_INVALID_WORK_DIMENSION", // -53
"CL_INVALID_WORK_GROUP_SIZE", // -54
"CL_INVALID_WORK_ITEM_SIZE", // -55
"CL_INVALID_GLOBAL_OFFSET", // -56
"CL_INVALID_EVENT_WAIT_LIST", // -57
"CL_INVALID_EVENT", // -58
"CL_INVALID_OPERATION", // -59
"CL_INVALID_GL_OBJECT", // -60
"CL_INVALID_BUFFER_SIZE", // -61
"CL_INVALID_MIP_LEVEL" // -62
};

void cl::reportStatus(const char * file,int line,const char * function,int status)
{
  if (status >= 0) return;
  status = -status;
#if CONFIG_USE_QT
  QString e;
  if (status >= NErrorCodes || ErrorCodes[status] == 0) e = QString("%1").arg(status);
  else e = ErrorCodes[status];
  qDebug() << QString("OpenCL ERROR %1(%2),%3: %4").arg(file).arg(line).arg(function).arg(e);
#else
  const char * e = 0;
  if (status >= NErrorCodes || ErrorCodes[status] == 0) e = "???";
  else e = ErrorCodes[status];
  printf("Error %s(%d),%s,%s\n",file,line,function,e);
#endif
#ifdef Linux
  // exit(1);
#endif
}
