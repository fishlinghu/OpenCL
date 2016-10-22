__kernel void inner_product( __global int* x, __global int* y, __global long int* stride, __global long int* num_of_elements )
	{
	long int idx = 0;
	__local long int i;
	long int max_steps = (*num_of_elements);
	long int num_of_strides = (*stride);
	for(i = 0; i < max_steps; ++i)
		{
		idx = i * num_of_strides;
		y[idx] = y[idx] * x[idx];
		}
	x[0] = idx;
	}

__kernel void null_loop( __global int* x, __global int* y, __global long int* stride, __global long int* num_of_elements )
	{
	long int idx = 0;
	__local long int i;
	long int max_steps = (*num_of_elements);
	long int num_of_strides = (*stride);
	for(i = 0; i < max_steps; ++i)
		{
		idx = i * num_of_strides;
		// y[idx] = y[idx] * x[idx];
		}
        x[0] = idx;
	}

//__kernel void multiply( __global long int* a, __global long int* b, __global long int* c, __global int* stride, __global unsigned long int* num_of_steps )
	//{
	/* code */
	//}
