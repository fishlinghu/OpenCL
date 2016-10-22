__kernel void stride_array(__global int* x, __global long int* stride, __global long int* num_of_steps )
	{
	long int temp = 0;
	__local long int i;
	long int nextstep = 0;
	long int max_steps = (*num_of_steps);
	//long int num_of_stride = (*stride);
	for(i = 0; i < max_steps; ++i)
		{
		temp += x[nextstep];
		nextstep = x[nextstep];
		}
	x[0] = temp;
	}

__kernel void stride_null_array(__global int* x, __global long int* stride, __global long int* num_of_steps )
	{
	long int temp = 0;
	__local long int i;
	long int nextstep = 0;
	long int max_steps = (*num_of_steps);
	//long int num_of_stride = (*stride);
	for(i = 0; i < max_steps; ++i)
		{
		//--nextstep;
		temp += i;
		//nextstep = x[nextstep];
		}
	x[0] = temp;
	}

__kernel void bw(__global unsigned char* src, __global unsigned char* dst)
	{
	__local int i;
	int xid = (get_global_id(0) + get_global_size(0) * get_global_id(1) + get_global_size(0) * get_global_size(1) * get_global_id(2)) * 64;
	dst[xid] = src[xid];
	for(i = 0; i < 64; ++i)
		{
		xid += i;
		dst[xid] = src[xid];
		}
	}

//__kernel void multiply( __global long int* a, __global long int* b, __global long int* c, __global int* stride, __global unsigned long int* num_of_steps )
	//{
	/* code */
	//}
