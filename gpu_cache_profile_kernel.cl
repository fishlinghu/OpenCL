__kernel void stride_array(__global int* x, __global unsigned long int* num_of_steps )
	{
	__local int temp;
	long int i;
	long int nextstep = 0;
	long int max_steps = (*num_of_steps);
	for(i = 0; i < max_steps; ++i)
		{
		temp = x[nextstep];
		nextstep = x[nextstep];
		}
	x[0] = temp;
	}

__kernel void stride_null_array(__global int* x, __global unsigned long int* num_of_steps )
	{
	long int i;
	long int nextstep = 0;
	long int max_steps = (*num_of_steps);
	for(i = 0; i < max_steps; ++i)
		{
		//--nextstep;
		nextstep = x[nextstep];
		}
	x[0] = i;
	}

//__kernel void multiply( __global long int* a, __global long int* b, __global long int* c, __global int* stride, __global unsigned long int* num_of_steps )
	//{
	/* code */
	//}
