__kernel void multiplier(__global const float* a, __global const float* b, __global float* result, __global const int* data_size)
	{
	float total = 0;
	int i = (*data_size) - 1;
	int idx = get_global_id(0);
	int idy = get_global_id(1);
	while (i >= 0)
		{
		total = total + a[idx][i] * b[i][idy];
		--i;
		}
	result[idx][idy] = total;
	}