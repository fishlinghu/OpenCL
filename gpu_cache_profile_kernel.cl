__kernel void stride_array(__global long int* x, __global int* stride, __global unsigned long int* num_of_steps )
	{
	long int i;
	long int nextstep = 0;
	long int max_steps = (*num_of_steps);
	for(i = 0; i < max_steps; ++i)
		{
		nextstep = x[nextstep];
		}
	x[0] = i;
	}

__kernel void stride_null_array( __global long int* x, __global int* stride, __global unsigned long int* num_of_steps )
	{
	long int i;
	long int nextstep = 0;
	long int max_steps = (*num_of_steps);
	for(i = 0; i < max_steps; ++i)
		{
		//--nextstep;
		nextstep = nextstep + (*stride);
		}
	x[0] = i;
	}

//__kernel void multiply( __global long int* a, __global long int* b, __global long int* c, __global int* stride, __global unsigned long int* num_of_steps )
	//{
	/* code */
	//}