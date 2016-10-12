// GPU multi-precision
// (c) EB Nov 2009

__kernel void mul1_v0(int k,__global const int * x,__global int * z)
{
  int i = get_global_id(0);
  z[i] = (int)(((long)k*(long)x[i]) & (long)BASE_MINUS1);
}

__kernel void mul1_v1(int k,__global const int * x,__global int * z)
{
  int i = get_global_id(0);
  long z0 = ((long)k*(long)x[i]) & (long)BASE_MINUS1;
  long z1 = (((long)k*(long)((i>0)?x[i-1]:0))>>LOG_BASE) & (long)BASE_MINUS1;
  long z2 = (((long)k*(long)((i>1)?x[i-2]:0))>>(2*LOG_BASE)) & (long)BASE_MINUS1;
  z[i] = (int)(z0+z1+z2);
}

#if 0
__kernel void mul1_v2(int k,__global const int * x,__global int * z)
{
  __local long aux[WORKGROUP_SIZE+2];
  int i = get_global_id(0); // in X
  int ii = get_local_id(0); // in workgroup
  long s;

  // Load X[i]*K into AUX for all threads of the workgroup + 2
  s = (long)x[i];
  aux[ii+2] = s * (long)k;
  if (ii < 2) // 2 work items load the previous 2 values
  {
    s = (long)((i>2)?x[i-2]:0);
    aux[ii] = s * (long)k;
  }
  barrier(CLK_LOCAL_MEM_FENCE);

  // Shift, mask and sum the 3 consecutive words in each cell
  s  = (aux[ii]>>(2*LOG_BASE)) & (long)BASE_MINUS1;
  s += (aux[ii+1]>>(LOG_BASE)) & (long)BASE_MINUS1;
  s +=  aux[ii+2]              & (long)BASE_MINUS1;

  // Store the result
  z[i] = (int)s;
}
#endif

#if 0
__kernel void mul1_v3(int k,__global const int * x,__global int * z)
{
  // Get index of block to compute
  int i = get_global_id(0) * BLOCK_SIZE;

  // Load the previous two values
  long p0,p1,p2;
  if (i>0)
  {
    p2 = (long)x[i-2]*(long)k; p2 >>= (2*LOG_BASE);
    p1 = (long)x[i-1]*(long)k; p1 >>= LOG_BASE;
  }
  else p2 = p1 = 0;

  // Compute the block (sequentially)
  for (int j=0;j<BLOCK_SIZE;j++)
  {
    // Load one value
    p0 = (long)x[i]*(long)k;
    // Store one result computed from the last 3 values
    z[i] = (int)((p0&(long)BASE_MINUS1)+(p1&(long)BASE_MINUS1)+p2);
    // Shift
    i++;
    p2 = p1 >> LOG_BASE;
    p1 = p0 >> LOG_BASE;
  }
}
#endif

__kernel void mul1_v3bis(int k,__global const int * x,__global int * z)
{
  int i = get_global_id(0) << 1;
  long m1,m2;
  if (i>0)
  {
    m2 = ((long)k*(long)x[i-2]) >> (2*LOG_BASE);
    m1 = ((long)k*(long)x[i-1]) >> LOG_BASE;
  } else m1 = m2 = 0;
  long x0 = (long)k*(long)x[i];
  long x1 = (long)k*(long)x[i+1];
  z[i]   = (int)( ( x0 & (long)BASE_MINUS1 )
                + ( m1 & (long)BASE_MINUS1 )
                + ( m2 & (long)BASE_MINUS1 ) );
  x0 >>= LOG_BASE;
  m1 >>= LOG_BASE;
  z[i+1] = (int)( ( x1 & (long)BASE_MINUS1 )
                + ( x0 & (long)BASE_MINUS1 )
                + ( m1 & (long)BASE_MINUS1 ) );
}

__kernel void mul1_v4a(int k,__global const int * x,__global int * z0,__global int * z1,__global int * z2)
{
  int i = get_global_id(0);
  long u = (long)k*(long)x[i];
  z0[i] = (int) (u & (long)BASE_MINUS1);
  u >>= LOG_BASE;
  z1[i] = (int) (u & (long)BASE_MINUS1);
  u >>= LOG_BASE;
  z2[i] = (int)u;
}

__kernel void mul1_v4b(__global const int * z0,__global const int * z1,__global const int * z2,__global int * z)
{
  int i = get_global_id(0);
  int s = z0[i];
  if (i>0) s += z1[i-1];
  if (i>1) s += z2[i-2];
  z[i] = s;
}
