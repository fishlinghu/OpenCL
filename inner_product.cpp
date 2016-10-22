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

void pattern_gen(long int* arr, long int stride, long int num_of_element)
    {
    // need to be newed outside of the function
    int i = 0;
    while( i < num_of_element * stride )
    	{
    	if( i % stride == 0 )
    		arr[i] = i + 1;
    	else
    		arr[i] = 0;
    	++i;
    	}
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
    //long int DATA_SIZE = atoi( argv[1] );
    long int NUM_OF_ACCESS = atoi( argv[1] );
    long long int SIZE_OF_DATA = atoi( argv[2] );
    // cout << 111111111111111 << endl;
    int i, j, k;
    char* info;
    size_t infoSize;
    cl_uint platformCount; // get the # of platforms available
    cl_platform_id *platforms;
    const char* attributeNames[5] = { "Name", "Vendor", "Version", "Profile", "Extensions" };
    // A platform only gets these 5 types of attributes, and we have to follow the naming rule
    const cl_platform_info attributeTypes[5] = { CL_PLATFORM_NAME, CL_PLATFORM_VENDOR, CL_PLATFORM_VERSION, CL_PLATFORM_PROFILE, CL_PLATFORM_EXTENSIONS };
    const int attributeCount = sizeof(attributeNames) / sizeof(char*);
 
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

   
    /* Initialize output */
    
    long int *arr;
    long int *arr2;
    cl_ulong time_start, time_end;
    double total_time;

    long long int csize;
    double loadtime, lastsec, sec0, sec1, sec; /* timing variables */
    cl_int stride;
    cl_ulong steps;

    printf(" ,");
    for (stride=1; stride <= ARRAY_MAX/2; stride=stride*2)
        label(stride*sizeof(int));
    printf("\n");
    

    /* Main loop for each configuration */
    //for (csize=ARRAY_MIN; csize <= ARRAY_MAX; csize=csize*2) 
        //{
        //label(csize*sizeof(int)); /* print cache size this loop */
        for (stride=1; stride <= 256; stride=stride*2) 
            {
    		//csize = 16;
    		//stride = 2;
            steps = NUM_OF_ACCESS;
            
            arr = new long int [steps * stride];
            pattern_gen( arr, stride, steps );

            arr2 = new long int [steps * stride];
            pattern_gen( arr2, stride, steps );

            

            /* Allocate memory and copy array to the device */
            cl_mem cl_x = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_long) * steps * stride, &arr[0], NULL);

          	cl_mem cl_y = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_long) * steps * stride, &arr2[0], NULL);

            cl_mem cl_stride = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &stride, NULL);

            cl_mem cl_steps = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_ulong), &steps, NULL);

            /* Load program and create kernel */
            cl_program program = load_program(context, "inner_product_kernel.cl");
            if(program == 0) 
                {
                cerr << "Can't load or build program\n";
                clReleaseMemObject(cl_x);
                clReleaseMemObject(cl_y);
                //clReleaseMemObject(cl_b);
                //clReleaseMemObject(cl_res);
                clReleaseCommandQueue(queue);
                clReleaseContext(context);
                return 0;
                }

            cl_kernel inner_product = clCreateKernel(program, "inner_product", 0);
            if(inner_product == 0) 
                {
                cerr << "Can't load kernel\n";
                clReleaseProgram(program);
                clReleaseMemObject(cl_x);
                clReleaseMemObject(cl_y);
                //clReleaseMemObject(cl_b);
                //clReleaseMemObject(cl_res);
                clReleaseCommandQueue(queue);
                clReleaseContext(context);
                return 0;
                }

            /* Pass argument to the device kernel */
            clSetKernelArg(inner_product, 0, sizeof(cl_mem), &cl_x);
            clSetKernelArg(inner_product, 1, sizeof(cl_mem), &cl_y);
            clSetKernelArg(inner_product, 2, sizeof(cl_mem), &cl_stride);
            clSetKernelArg(inner_product, 3, sizeof(cl_mem), &cl_steps);

            cl_kernel null_loop = clCreateKernel(program, "null_loop", 0);
            if(null_loop == 0) 
                {
                cerr << "Can't load kernel\n";
                clReleaseProgram(program);
                clReleaseMemObject(cl_x);
                clReleaseMemObject(cl_y);
                //clReleaseMemObject(cl_b);
                //clReleaseMemObject(cl_res);
                clReleaseCommandQueue(queue);
                clReleaseContext(context);
                return 0;
                }

            /* Pass argument to the device kernel */
            clSetKernelArg(null_loop, 0, sizeof(cl_mem), &cl_x);
            clSetKernelArg(null_loop, 1, sizeof(cl_mem), &cl_y);
            clSetKernelArg(null_loop, 2, sizeof(cl_mem), &cl_stride);
            clSetKernelArg(null_loop, 3, sizeof(cl_mem), &cl_steps);

            /* Execution of the kernel */
            size_t work_size = 1;
            cl_int err;
            cl_event event;

            clFinish(queue);
            lastsec = gettime();
            do sec0 = gettime(); while (sec0 == lastsec);

            err = clEnqueueNDRangeKernel(queue, inner_product, 1, 0, &work_size, 0, 0, 0, &event);
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

            ////////Run the NULL loop

            clFinish(queue);
            lastsec = gettime();
            do sec0 = gettime(); while (sec0 == lastsec);

            err = clEnqueueNDRangeKernel(queue, null_loop, 1, 0, &work_size, 0, 0, 0, &event);
            clWaitForEvents(1 , &event);
            sec1 = gettime(); /* end timer */

            sec = sec - (sec1 - sec0);

            clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
            clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
            total_time = total_time - (time_end - time_start);

            //printf("\nExecution time in milliseconds = %0.3f ms\n", (total_time / 1000000.0) );



            //loadtime = (sec*1e9)/steps;
            loadtime = (total_time)/steps;
            /* write out results in .csv format for Excel */
            printf("%4.1f,", (loadtime<0.1) ? 0.1 : loadtime);

            /*if(err == CL_SUCCESS) 
                {
                err = clEnqueueReadBuffer(queue, cl_a, CL_TRUE, 0, sizeof(long int) * DATA_SIZE, &a[0], 0, 0, 0);
                }*/

            
            //cout << "Err code: " << err << endl;

            clReleaseKernel(inner_product);
            clReleaseKernel(null_loop);
            clReleaseProgram(program);
            clReleaseMemObject(cl_x);
            clReleaseMemObject(cl_y);
            clReleaseMemObject(cl_stride);
            clReleaseMemObject(cl_steps);
            //clReleaseCommandQueue(queue);

            delete [] arr;
            delete [] arr2;

            }; /* end of inner for loop */
        printf("\n");
        //}; /* end of outer for loop */

    /////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////// 


    clReleaseContext(context);

    ////
    free(platforms);
    return 0;
	}