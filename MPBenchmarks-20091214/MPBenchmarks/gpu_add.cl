// GPU addition
// (c) EB Sep 2009

#if (WORD_SIZE == 64)

typedef long Word;
const Word LOG_BASE = 62;
const Word BASE = (Word)1<<(Word)62;
const Word BASE_MINUS1 = (Word)1<<(Word)62 - 1;

#elif (WORD_SIZE == 32)

typedef int Word;
const Word LOG_BASE = 30;
const Word BASE = (Word)1<<(Word)30;
const Word BASE_MINUS1 = (Word)1<<(Word)30 - 1;

#elif (WORD_SIZE == 16)

typedef short Word;
const Word LOG_BASE = 14;
const Word BASE = (Word)1<<(Word)14;
const Word BASE_MINUS1 = (Word)1<<(Word)14 - 1;

#endif

__kernel void add_v1(__global const Word * x,__global const Word * y,__global Word * z)
{
  int i = get_global_id(0);

  // Carry from I-1
  Word t = (Word)0;
  if (i>0)
  {
    t = (x[i-1]+y[i-1]) >> LOG_BASE;
  }

  // Sum I
  Word s = (x[i]+y[i]) & BASE_MINUS1;

  // Final result
  z[i] = t + s;
}

__kernel void add_v2(__global const Word * x,__global const Word * y,__global Word * z)
{
  int i = get_global_id(0);
  z[i] = x[i] + y[i];
}

#if 0
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
#endif
