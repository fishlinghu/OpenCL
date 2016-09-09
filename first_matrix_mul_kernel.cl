/*__kernel void multiplier(__global const float* a, __global const float* b, __global float* result, __global const cl_int* data_size)
	{
	float total = 0;
	int i = (*data_size) - 1;
	int idx = get_global_id(0);
	int idy = get_global_id(1);
	int idx_offset = idx * (*data_size);
	while (i >= 0)
		{
		total = total + a[ idx_offset + i ] * b[ i*(*data_size) + idy ];
		--i;
		}
	result[idx_offset + idy] = total;
	}*/

__kernel void multiplier(__global const float* a, __global const float* b, __global float* result)
	{
	float total = 0;
	int i = 100 - 1;
	int idx = get_global_id(0);
	int idy = get_global_id(1);
	int idx_offset = idx * 100;
	while (i >= 0)
		{
		total = total + a[ idx_offset + i ] * b[ i*100 + idy ];
		--i;
		}
	result[idx_offset + idy] = total;
	}