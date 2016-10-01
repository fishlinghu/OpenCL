__kernel void multiply(__global const float* a, __global const float* b, __global float* result, __global const long int* data_size)
	{
	int idx = get_global_id(0);
	result[idx] = a[idx] + b[idx];
	}