__kernel void stride_array(__global long int* x, __global int* stride, __global long int* num_of_steps )
	{
	//int idx = get_global_id(0);
	//result[idx] = a[idx] + b[idx];
	long int i;
	int j, nextstep;
	long int max_steps = (*num_of_steps);
	for(i = 0; i < max_steps; ++i)
		{
		for (j = stride; j > 0; --j) 
            { /* keep samples same */
            nextstep = 0;
            //do nextstep = x[nextstep]; /* dependency */
            //while (nextstep != 0);
            }
		}
	}