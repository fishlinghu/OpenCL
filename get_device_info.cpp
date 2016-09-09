#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define DATA_SIZE 1000000

using namespace std;

cl_program load_program(cl_context context, const char* filename)
    {
    ifstream in(filename, ios_base::binary);
    if(!in.good()) 
        {
        return 0;
        }

    // get file length
    in.seekg(0, ios_base::end);
    size_t length = in.tellg();
    in.seekg(0, ios_base::beg);

    // read program source
    vector<char> data(length + 1);
    in.read(&data[0], length);
    data[length] = 0;

    // create and build program 
    const char* source = &data[0];
    cl_program program = clCreateProgramWithSource(context, 1, &source, 0, 0);
    if(program == 0) 
        return 0;

    if(clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS) 
        return 0;

    return program;
    }

int main()
    {
    cl_int err;
    cl_uint num;
    err = clGetPlatformIDs(0, 0, &num);
    if(err != CL_SUCCESS) 
        {
        cerr << "Unable to get platforms\n";
        return 0;
        }

    vector<cl_platform_id> platforms(num);
    err = clGetPlatformIDs(num, &platforms[0], &num);
    if(err != CL_SUCCESS) 
        {
        std::cerr << "Unable to get platform ID\n";
        return 0;
        }
    // Create context
    cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[0]), 0 };
    cl_context context = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);
    if(context == 0) 
        {
        std::cerr << "Can't create OpenCL context\n";
        return 0;
        }

    // Get the Info of Devices
    size_t cb;
    clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
    vector<cl_device_id> devices(cb / sizeof(cl_device_id));
    clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, &devices[0], 0);

    clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &cb);
    string devname;
    devname.resize(cb);
    clGetDeviceInfo(devices[0], CL_DEVICE_NAME, cb, &devname[0], 0);
    cout << "Device: " << devname.c_str() << "\n";

    cl_command_queue queue = clCreateCommandQueue(context, devices[0], 0, 0);
    if(queue == 0) 
        {
        cerr << "Can't create command queue\n";
        clReleaseContext(context);
        return 0;
        }

    // Initialization of data
    vector<float> a(DATA_SIZE), b(DATA_SIZE), res(DATA_SIZE);
    for(int i = 0; i < DATA_SIZE; i++) 
        {
        a[i] = rand();
        b[i] = rand();
        }

    // Allocating memory at GPU
    cl_mem cl_a = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * DATA_SIZE, &a[0], NULL);
    cl_mem cl_b = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * DATA_SIZE, &b[0], NULL);
    cl_mem cl_res = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * DATA_SIZE, NULL, NULL);
    if(cl_a == 0 || cl_b == 0 || cl_res == 0) 
        {
        cerr << "Can't create OpenCL buffer\n";
        clReleaseMemObject(cl_a);
        clReleaseMemObject(cl_b);
        clReleaseMemObject(cl_res);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 0;
       }

    cl_program program = load_program(context, "shader.cl");
    if(program == 0) 
        {
        cerr << "Can't load or build program\n";
        clReleaseMemObject(cl_a);
        clReleaseMemObject(cl_b);
        clReleaseMemObject(cl_res);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 0;
        }
    // Access the adder function
    cl_kernel adder = clCreateKernel(program, "adder", 0);
    if(adder == 0) 
        {
        cerr << "Can't load kernel\n";
        clReleaseProgram(program);
        clReleaseMemObject(cl_a);
        clReleaseMemObject(cl_b);
        clReleaseMemObject(cl_res);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 0;
        }

    clSetKernelArg(adder, 0, sizeof(cl_mem), &cl_a);
    clSetKernelArg(adder, 1, sizeof(cl_mem), &cl_b);
    clSetKernelArg(adder, 2, sizeof(cl_mem), &cl_res);

    size_t work_size = DATA_SIZE;
    err = clEnqueueNDRangeKernel(queue, adder, 1, 0, &work_size, 0, 0, 0, 0);
    if(err==CL_SUCCESS)
        clEnqueueReadBuffer(queue, cl_res, CL_TRUE, 0, sizeof(float) * DATA_SIZE, &res[0], 0, 0, 0);    

    if(err == CL_SUCCESS) 
        {
        bool correct = true;
        for(int i = 0; i < DATA_SIZE; i++) 
            {
            if(a[i] + b[i] != res[i]) 
                {
                correct = false;
                break;
                }
            }

    if(correct) 
        cout << "Data is correct\n";
    else 
        cout << "Data is incorrect\n";

        }
    else 
        cerr << "Can't run kernel or read back data\n";
    

    clReleaseKernel(adder);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
    }