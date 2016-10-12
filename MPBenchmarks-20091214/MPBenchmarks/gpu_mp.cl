// GPU multi-precision (old file)
// (c) EB Sep 2009

#if (WORD_SIZE == 64)

typedef long Word;
const Word BASE = (Word)1<<(Word)60;
const Word MAX_WORD = (Word)1<<(Word)60 - (Word)1;

#elif (WORD_SIZE == 32)

typedef int Word;
const Word BASE = (Word)1<<(Word)30;
const Word MAX_WORD = (Word)1<<(Word)30 - (Word)1;

#elif (WORD_SIZE == 16)

typedef short Word;
const Word BASE = (Word)1<<(Word)14;
const Word MAX_WORD = (Word)1<<(Word)14 - (Word)1;

#elif (WORD_SIZE == 128)

typedef float4 Word;

#endif

__kernel void copy(__global Word * x,__global Word * z)
{
  int i = get_global_id(0);
  z[i] = x[i];
}

__kernel void zero(__global Word * a)
{
  int i = get_global_id(0);
  a[i] = 0.0f;
}

__kernel void add_v1(__global const Word * x,__global const Word * y,__global Word * z)
{
  int i = get_global_id(0);

  // Carry from I-1
  Word t = (Word)0;
  if (i>0)
  {
    Word s1 = x[i-1]+y[i-1];
    if (s1 >= MAX_WORD) t = 1;
    else if (s1 <= -MAX_WORD) t = -1;
  }

  // Sum I
  Word s = x[i]+y[i];
  if (s >= MAX_WORD) s -= BASE;
  else if (s <= -MAX_WORD) s += BASE;

  // Final result
  z[i] = t + s;
}

__kernel void add_v3(__global const Word * x,__global const Word * y,__global Word * z,
		   __local Word * sLoc)
{
  int i = get_global_id(0);
  int ii = get_local_id(0);
  sLoc[ii] = x[i] + y[i];

  barrier(CLK_LOCAL_MEM_FENCE);

  // Carry from I-1
  Word t = (Word)0;
  Word s = 0;
#if 0
  if (ii == 0)
  {
    s = (i>0)?(x[i-1] + y[i-1]):0;
  }
  else
  {
    // s = sLoc[ii-1];
  }
#endif
  // s = sLoc[ii-1];
  if (s >= MAX_WORD) t = 1;
  else if (s <= -MAX_WORD) t = -1;

  // Sum I
  s = sLoc[ii-1];
  if (s >= MAX_WORD) s -= BASE;
  else if (s <= -MAX_WORD) s += BASE;

  // Final result
  z[i] = t + s;
}

__kernel void add_v2(__global const Word * x,__global const Word * y,__global Word * z)
{
  int i = get_global_id(0);
  z[i] = x[i] + y[i];
}

__kernel void propagate(__global const Word * x,__global Word * z)
{
  int i = get_global_id(0);
  Word a = x[i];
  Word b = 0;
}

// ______________________________________________________________________
//

#if (VECTOR_LENGTH == 1)
typedef float vector_t;
#elif (VECTOR_LENGTH == 2)
typedef float2 vector_t;
#elif (VECTOR_LENGTH == 4)
typedef float4 vector_t;
#endif

__kernel void crunch(__global float * out)
{
  vector_t x,y,cs,sn,xx,yy;
  x = 1.0f;
  y = 0.0f;
  cs = cos(2.0f); // random angle
  sn = sin(2.0f);
  for (int i=0;i<N_ROTATIONS;i++)
  {
    xx = x*cs - y*sn;
    yy = y*cs + x*sn;
    x = xx;
    y = yy;
  }
  out[get_global_id(0)] = dot(x,y);
}

__kernel void crunch2(__global float * out)
{
  vector_t x,y,cs,sn,xx,yy;
  x = 1.0f;
  y = 0.0f;
  cs = cos(2.0f); // random angle
  sn = sin(2.0f);
  for (int i=0;i<N_ROTATIONS;i++)
  {
    xx = mad(x,cs,-y*sn);
    yy = mad(y,cs,x*sn);
    x = xx;
    y = yy;
  }
  out[get_global_id(0)] = dot(x,y);
}
