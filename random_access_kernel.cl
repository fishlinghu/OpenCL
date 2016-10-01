__kernel void random_access(__global long int* a, __global const long int* data_size)
	{
	//int idx = get_global_id(0);
	//result[idx] = a[idx] + b[idx];
	int i;
	long int next = 0;
	for(i = 0; i < (*data_size); ++i)
		{
		next = a[ next ];
		}
	}