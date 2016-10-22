#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <CL/cl.hpp>

#define MSTRINGIFY(...) #__VA_ARGS__

#define KB 1024
#define MB 1024 * KB
#define SIZE 38 * MB

using namespace std;

static const char *sKernels =
    #include "kernel.cl"
    ;
  
float timeDiff(cl::Event &timeEvent)
{
  cl_ulong start = timeEvent.getProfilingInfo<CL_PROFILING_COMMAND_START>() / 1000;
  cl_ulong end = timeEvent.getProfilingInfo<CL_PROFILING_COMMAND_END>() / 1000;

  return (float)((int)end - (int)start);
}

// Function ack: Taken from OPenCL peak repo
float run_kernel(cl::CommandQueue &queue, cl::Kernel &kernel, cl::NDRange &globalSize, cl::NDRange &localSize, int iters)
{
  float timed = 0;

  // Dummy calls
  queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalSize, localSize);
  queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalSize, localSize);
  queue.finish();


  for(int i=0; i<iters; i++)
  {
    cl::Event timeEvent;

    queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalSize, localSize, NULL, &timeEvent);
    queue.finish();
    timed += timeDiff(timeEvent);
  } 
  return (timed / iters);
}

int main(int argc, char ** argv)
{
  vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);
  string plaformName = platforms[0].getInfo<CL_PLATFORM_NAME>();
  cl_context_properties cps[3] = {
  CL_CONTEXT_PLATFORM,
  (cl_context_properties)(platforms[0])(),
  0
  };
  cl_device_type device_type = CL_DEVICE_TYPE_GPU;
  cl::Context ctx(device_type, cps);
  vector<cl::Device> devices = ctx.getInfo<CL_CONTEXT_DEVICES>();
  cl::Program prog;
  cl::Program::Sources source(1, make_pair(sKernels, (strlen(sKernels)+1)));
  
  prog = cl::Program(ctx, source);
  
  for(int d=0; d < (int)devices.size(); d++)
  {
    
    vector<cl::Device> dev = {devices[d]};
    prog.build(dev);
    cout << "Build Log:\t " << prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;
    
    cl::CommandQueue queue = cl::CommandQueue(ctx, devices[d], CL_QUEUE_PROFILING_ENABLE);
	  
    float timed, gflops;
    cl_uint workPerWI;
    cl::NDRange computeSize, wg_items;
    cl_int lengthMod;
    int iters = 2; 

    cl::Buffer outputBuf = cl::Buffer(ctx, CL_MEM_READ_WRITE, (40*SIZE/sizeof(cl_int)));

    computeSize = 1;
    wg_items = 1;

    int sizes[] = {
                1 * KB, 4 * KB, 8 * KB, 16 * KB, 32 * KB, 64 * KB, 128 * KB, 256 * KB,
        512 * KB, 1 * MB, 2 * MB, 4 * MB, 8 * MB, 16 * MB, 32 * MB
    };

    for (int i = 0; i < sizeof(sizes)/sizeof(int); i++) {
        lengthMod = sizes[i]/sizeof(int) - 1;
        cl::Kernel kernel_v1(prog, "cache_access");
        kernel_v1.setArg(0, outputBuf), kernel_v1.setArg(1, lengthMod);

        timed = run_kernel(queue, kernel_v1, computeSize, wg_items, iters);

        printf("Stride size %d KB : %f\n", sizes[i] / (1 * KB), timed/1000000);
    }

      
  }
   
}