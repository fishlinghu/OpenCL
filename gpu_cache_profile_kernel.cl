__kernel void stride_array(__global long int* x, __global int* stride, __global long int* num_of_steps )
	{
	long int i;
	long int nextstep = 0;
	long int max_steps = (*num_of_steps);
	for(i = 0; i < max_steps; ++i)
		{
		nextstep = x[nextstep];
		}
	}

__kernel void stride_null_array(__global long int* x, __global int* stride, __global long int* num_of_steps )
	{
	long int i;
	long int nextstep = 0;
	long int max_steps = (*num_of_steps);
	for(i = 0; i < max_steps; ++i)
		{
		++nextstep;
		}
	}