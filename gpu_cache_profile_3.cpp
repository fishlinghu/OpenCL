#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sys/time.h>
#include <time.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

#define ARRAY_MIN (1024) /* 1/4 smallest cache */
#define ARRAY_MAX (2048*2048) /* 1/4 largest cache */

struct timespec kernel_start_time;
struct timespec kernel_end_time;

double gettime() 
    {
    struct timeval t;
    gettimeofday(&t,NULL);
    return t.tv_sec+t.tv_usec*1e-6;
    }

int label(int i) 
    {/* generate text labels */
    if (i<1e3) 
        printf("%1dB,",i);
    else if (i<1e6) 
        printf("%1dK,",i/1024);
    else if (i<1e9) 
        printf("%1dM,",i/1048576);
    else 
        printf("%1dG,",i/1073741824);
    return 0;
    }

void print_timer_message(timespec start_time, timespec end_time)
    {
    printf("\n***************** Total time *****************\n");
    printf("s_time.tv_sec:%ld, s_time.tv_nsec:%09ld\n", start_time.tv_sec, start_time.tv_nsec);
    printf("e_time.tv_sec:%ld, e_time.tv_nsec:%09ld\n", end_time.tv_sec, end_time.tv_nsec);
    if(end_time.tv_nsec > start_time.tv_nsec)
        {
        printf("[diff_time:%ld.%09ld sec]\n",
        end_time.tv_sec - start_time.tv_sec,
        end_time.tv_nsec - start_time.tv_nsec);
        }
    else
        {
        printf("[diff_time:%ld.%09ld sec]\n",
        end_time.tv_sec - start_time.tv_sec - 1,
        end_time.tv_nsec - start_time.tv_nsec + 1000*1000*1000);
        }
    return;
    }

void pattern_gen(long int* arr, long int stride, long int size_of_arr)
    {
    // need to be newed outside of the function
    int i = 0;
    while( i < size_of_arr )
    	{
    	//if( i % stride == 0 )
    		arr[i] = i + stride;
    	//else
    		//arr[i] = 0;
        //cout << arr[i] << endl;
    	++i;
    	}
    arr[ size_of_arr-stride ] = 0;
    return;
    }

cl_program load_program(cl_context context, const char* filename)
    {
    ifstream in(filename, std::ios_base::binary);
    if(!in.good()) 
        {
        return 0;
        }

    // get file length
    in.seekg(0, std::ios_base::end);
    size_t length = in.tellg();
    in.seekg(0, std::ios_base::beg);

    // read program source
    vector<char> data(length + 1);
    in.read(&data[0], length);
    data[length] = 0;

    // create and build program 
    const char* source = &data[0];
    cl_program program = clCreateProgramWithSource(context, 1, &source, 0, 0);
    if(program == 0) 
        {
        return 0;
        }

    if(clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS) 
        {
        return 0;
        }

    size_t size;
    cl_int status;
    status = clGetProgramInfo( program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &size, NULL);

    unsigned char * binary = new unsigned char [ size ];
    status = clGetProgramInfo( program, CL_PROGRAM_BINARIES, size, &binary, NULL ); 

    /*int i = 0;
    while(i < size)
        {
        cout << binary[i];
        ++i;
        }
    cout << endl;*/

    delete [] binary;

    return program;
    }


int main(int argc, char* argv[]) 
    {
    srand( time(NULL) );
    long int NUM_OF_ACCESS = atoi( argv[1] );
    long long int SIZE_OF_DATA = atoi( argv[2] );
    
    cl_uint platformCount; // get the # of platforms available
    cl_platform_id *platforms;
 
    // get platform count
    clGetPlatformIDs(5, NULL, &platformCount);
    // get all platforms
    platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
    clGetPlatformIDs(platformCount, platforms, NULL);

    cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[0]), 0 };
    cl_context context = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);
    if(context == 0) 
        {
        cerr << "Can't create OpenCL context\n";
        return 0;
        }

    size_t cb;
    clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
    vector<cl_device_id> devices_for_compute(cb / sizeof(cl_device_id));
    clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, &devices_for_compute[0], 0);

    clGetDeviceInfo(devices_for_compute[0], CL_DEVICE_NAME, 0, NULL, &cb);
    string devname;
    devname.resize(cb);
    clGetDeviceInfo(devices_for_compute[0], CL_DEVICE_NAME, cb, &devname[0], 0);
    //cout << "Device: " << devname.c_str() << "\n";

    cl_command_queue queue = clCreateCommandQueue(context, devices_for_compute[0], CL_QUEUE_PROFILING_ENABLE, 0);
    if(queue == 0) 
        {
        cerr << "Can't create command queue\n";
        clReleaseContext(context);
        return 0;
        }


    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    cl_ulong time_start, time_end;
    double total_time;
    double loadtime, lastsec, sec0, sec1, sec; /* timing variables */

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    
    cl_mem cmPinnedBufIn = NULL; 
    cl_mem cmPinnedBufOut = NULL; 
    cl_mem cmDevBufIn = NULL; 
    cl_mem cmDevBufOut = NULL; 
    unsigned char* cDataIn = NULL; 
    unsigned char* cDataOut = NULL; 

    cmPinnedBufIn = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, SIZE_OF_DATA, NULL, NULL); 
    cmPinnedBufOut = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, SIZE_OF_DATA, NULL, NULL);
    cmDevBufIn= clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE_OF_DATA, NULL, NULL); 
    cmDevBufOut= clCreateBuffer(context, CL_MEM_WRITE_ONLY, SIZE_OF_DATA, NULL, NULL);

    cDataIn = (unsigned char*)clEnqueueMapBuffer(queue, cmPinnedBufIn, CL_TRUE, CL_MAP_WRITE, 0, SIZE_OF_DATA, 0, NULL, NULL, NULL);
    cDataOut = (unsigned char*)clEnqueueMapBuffer(queue, cmPinnedBufOut, CL_TRUE, CL_MAP_READ, 0, SIZE_OF_DATA, 0, NULL, NULL, NULL);

    cDataIn = (unsigned char*) malloc(sizeof(unsigned char) * SIZE_OF_DATA);

    int i;

    for( i = 0; i < SIZE_OF_DATA; ++i)
        {
        cDataIn[i] = (unsigned char)(i & 0xff); 
        }

    double trans_start, trans_end;
    trans_start = gettime();
    clEnqueueWriteBuffer(queue, cmDevBufIn, CL_TRUE, 0, SIZE_OF_DATA, cDataIn, 0, NULL, NULL);
    trans_end = gettime();

    cl_program program = load_program(context, "gpu_cache_profile_kernel.cl");
    if(program == 0) 
        {
        cerr << "Can't load or build program\n";
        clReleaseMemObject(cmPinnedBufIn);
        clReleaseMemObject(cmPinnedBufOut);
        clReleaseMemObject(cmDevBufIn);
        clReleaseMemObject(cmDevBufOut);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 0;
        }

    cl_kernel bw = clCreateKernel(program, "bw", 0);
    if(bw == 0) 
        {
        cerr << "Can't load kernel\n";
        clReleaseProgram(program);
        clReleaseMemObject(cmPinnedBufIn);
        clReleaseMemObject(cmPinnedBufOut);
        clReleaseMemObject(cmDevBufIn);
        clReleaseMemObject(cmDevBufOut);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 0;
        }

    /* Pass argument to the device kernel */
    clSetKernelArg(bw, 0, sizeof(cl_mem), &cDataIn);
    clSetKernelArg(bw, 1, sizeof(cl_mem), &cDataOut);

	cl_ulong local_mem_size;
  	clGetKernelWorkGroupInfo(bw, devices_for_compute[0], CL_KERNEL_LOCAL_MEM_SIZE, sizeof(local_mem_size), &local_mem_size, NULL);
  	printf("  CL_KERNEL_LOCAL_MEM_SIZE\t%u\n", local_mem_size);

	//CL_DEVICE_MAX_WORK_ITEM_SIZES
     	size_t workitem_size[3];
       	clGetKernelWorkGroupInfo(bw, devices_for_compute[0], CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(workitem_size), &workitem_size, NULL);
        printf("  CL_KERNEL_COMPILE_WORK_GROUP_SIZES:\t%u / %u / %u \n", workitem_size[0], workitem_size[1], workitem_size[2]);
  
        // CL_DEVICE_MAX_WORK_GROUP_SIZE
        size_t workgroup_size;
        clGetKernelWorkGroupInfo(bw, devices_for_compute[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(workgroup_size), &workgroup_size, NULL);
        printf("  CL_KERNEL_WORK_GROUP_SIZE:\t%u\n", workgroup_size);
                 
    size_t global_work_size[3] = {256, 256, 256};
    size_t local_work_size[3] = {1, 1, 1};
    cl_int err;
    cl_event event;

    clFinish(queue);
    lastsec = gettime();
    do sec0 = gettime(); while (sec0 == lastsec);
	//cout << 1111 << endl;
    err = clEnqueueNDRangeKernel(queue, bw, 3, NULL, global_work_size, NULL, 0, 0, &event);
    //clWaitForEvents(1 , &event);
    	clFinish(queue);
	sec1 = gettime(); /* end timer */
    //err = clEnqueueReadBuffer(queue, cl_x, CL_TRUE, 0, sizeof(cl_long) * csize, &x[0], 0, 0, 0);
    //cout << x[0] << endl;
	//cout << 22222 << endl;
    sec = sec1 - sec0;
    //loadtime = (sec*1e9)/(steps*csize);

    //clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    //clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
    //total_time = time_end - time_start;


    clReleaseKernel(bw);
    clReleaseProgram(program);
    clReleaseMemObject(cmPinnedBufIn);
    clReleaseMemObject(cmPinnedBufOut);
    clReleaseMemObject(cmDevBufIn);
    clReleaseMemObject(cmDevBufOut);

    cout << "Data Size: " << SIZE_OF_DATA << " bytes" << endl;
    cout << "WriteBuffer Time: " << (trans_end - trans_start)*1e9 << " ns" << endl;
    //cout << "Copy Time: " << total_time << " ns" << endl;
    cout << "Copy Time: " << sec << " sec" << endl;
    cout << "Transfer Rate: " << SIZE_OF_DATA / 1e9 / sec << " GB/s" << endl;

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

        /* Initialize output */
    
    long int *arr;
    long long int csize;
    
    cl_int stride;
    cl_ulong steps;

    printf(" ,");
    for (stride=1; stride <= ARRAY_MAX/2; stride=stride*2)
        label(stride*sizeof(int));
    printf("\n");
    

    /* Main loop for each configuration */
    for (csize=ARRAY_MIN; csize <= ARRAY_MAX; csize=csize*2) 
        {
        label(csize*sizeof(int)); /* print cache size this loop */
        //for (stride=1; stride <= csize/2; stride=stride*2)
	for (stride=1; stride <= 1; stride=stride*2) 
            {
            arr = new long int [csize];
            pattern_gen( arr, stride, csize );

            /* Allocate memory and copy array to the device */
            cl_mem cl_x = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_long) * csize, &arr[0], NULL);

            cl_mem cl_stride = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &stride, NULL);

            steps = NUM_OF_ACCESS;

            cl_mem cl_steps = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_ulong), &steps, NULL);

            /* Load program and create kernel */
            cl_program program = load_program(context, "gpu_cache_profile_kernel.cl");
            if(program == 0) 
                {
                cerr << "Can't load or build program\n";
                clReleaseMemObject(cl_x);
                //clReleaseMemObject(cl_b);
                //clReleaseMemObject(cl_res);
                clReleaseCommandQueue(queue);
                clReleaseContext(context);
                return 0;
                }

            cl_kernel stride_array = clCreateKernel(program, "stride_array", 0);
            if(stride_array == 0) 
                {
                cerr << "Can't load kernel\n";
                clReleaseProgram(program);
                clReleaseMemObject(cl_x);
                //clReleaseMemObject(cl_b);
                //clReleaseMemObject(cl_res);
                clReleaseCommandQueue(queue);
                clReleaseContext(context);
                return 0;
                }

            /* Pass argument to the device kernel */
            clSetKernelArg(stride_array, 0, sizeof(cl_mem), &cl_x);
            clSetKernelArg(stride_array, 1, sizeof(cl_mem), &cl_stride);
            clSetKernelArg(stride_array, 2, sizeof(cl_mem), &cl_steps);

            cl_kernel stride_null_array = clCreateKernel(program, "stride_null_array", 0);
            if(stride_null_array == 0) 
                {
                cerr << "Can't load kernel\n";
                clReleaseProgram(program);
                clReleaseMemObject(cl_x);
                //clReleaseMemObject(cl_b);
                //clReleaseMemObject(cl_res);
                clReleaseCommandQueue(queue);
                clReleaseContext(context);
                return 0;
                }

            /* Pass argument to the device kernel */
            clSetKernelArg(stride_null_array, 0, sizeof(cl_mem), &cl_x);
            clSetKernelArg(stride_null_array, 1, sizeof(cl_mem), &cl_stride);
            clSetKernelArg(stride_null_array, 2, sizeof(cl_mem), &cl_steps);
            //clSetKernelArg(stride_array, 3, sizeof(cl_mem), &cl_nextstep);

            /* Execution of the kernel */
            size_t work_size = 1;
            cl_int err;
            cl_event event;

            clFinish(queue);
            lastsec = gettime();
            do sec0 = gettime(); while (sec0 == lastsec);

            err = clEnqueueNDRangeKernel(queue, stride_array, 1, 0, &work_size, 0, 0, 0, &event);
            clWaitForEvents(1 , &event);
            sec1 = gettime(); /* end timer */
            //err = clEnqueueReadBuffer(queue, cl_x, CL_TRUE, 0, sizeof(cl_long) * csize, &x[0], 0, 0, 0);
            //cout << x[0] << endl;

            sec = sec1 - sec0;
            //loadtime = (sec*1e9)/(steps*csize);

            clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
            clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
            total_time = time_end - time_start;
            //printf("\nExecution time in milliseconds = %0.3f ms\n", (total_time / 1000000.0) );

            clFinish(queue);
            lastsec = gettime();
            do sec0 = gettime(); while (sec0 == lastsec);

            err = clEnqueueNDRangeKernel(queue, stride_null_array, 1, 0, &work_size, 0, 0, 0, &event);
            clWaitForEvents(1 , &event);
            sec1 = gettime(); /* end timer */

            sec = sec - (sec1 - sec0);

            clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
            clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
            //total_time = time_end - time_start;
            total_time = total_time - (time_end - time_start);

            //printf("\nExecution time in milliseconds = %0.3f ms\n", (total_time / 1000000.0) );



            loadtime = (sec*1e9)/steps;
            //loadtime = (total_time)/steps;
            /* write out results in .csv format for Excel */
            printf("%4.1f,", (loadtime<0.1) ? 0.1 : loadtime);

            /*if(err == CL_SUCCESS) 
                {
                err = clEnqueueReadBuffer(queue, cl_a, CL_TRUE, 0, sizeof(long int) * DATA_SIZE, &a[0], 0, 0, 0);
                }*/

            
            //cout << "Err code: " << err << endl;

            clReleaseKernel(stride_array);
            clReleaseKernel(stride_null_array);
            clReleaseProgram(program);
            clReleaseMemObject(cl_x);
            clReleaseMemObject(cl_stride);
            clReleaseMemObject(cl_steps);
            //clReleaseCommandQueue(queue);

            delete [] arr;

            }; /* end of inner for loop */
        printf("\n");
        }; /* end of outer for loop */

    /////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////// 


    clReleaseContext(context);

    ////
    free(platforms);
    return 0;

    }
