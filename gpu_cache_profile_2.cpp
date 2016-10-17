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

void pattern_gen()
    {
    
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