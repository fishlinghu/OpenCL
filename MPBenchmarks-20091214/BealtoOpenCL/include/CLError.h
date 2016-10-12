// OpenCL error reporting
// (c) EB Sep 2009

#ifndef CLError_h
#define CLError_h

namespace cl {

// Report an OpenCL call status.
// FILE,LINE,FUNCTION shall be the source file/line/name of the function where the call failed.
// STATUS is the returned value (CL_SUCCESS,...).
void reportStatus(const char * file,int line,const char * function,int status);

// Use this macro to report errors.
#define REPORT_OPENCL_STATUS(status) { cl::reportStatus(__FILE__,__LINE__,__FUNCTION__,status); }

} // namespace

#endif // CLError_h
