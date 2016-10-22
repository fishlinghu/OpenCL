MSTRINGIFY(

\n  #pragma OPENCL EXTENSION cl_khr_fp64 : enable
\n


\n
\n#define KB 1024 // 1 KB = 1024 bytes
\n#define MB 1024 * KB // 1 MB = 1024 KB
\n#define MAX_SIZE 34 * MB // size of data array
\n#define REPS 1 *MB // times to access memory (MB/KB just used as millions/thousands multiplier)
\n#define TIMES 10 // times to repeat experiment to get "average"
\n
\n

__kernel void cache_access(__global float *ptr, int _A)
{
    int lengthMod = _A;
    float y = (float)get_local_id(0);

    for (int i = 0; i < MAX_SIZE; i++) {
        ptr[i] = i;
    }

    unsigned long long tmp = 0;

   // repeatedly read data
   for (int j = 0; j < TIMES; j++) {
        for (unsigned int k = 0; k < REPS; k++) {
            tmp += (ptr[(k * 64) & lengthMod]);
        } 
   }

    ptr[get_global_id(0)] = tmp;
}

\n
\n
)