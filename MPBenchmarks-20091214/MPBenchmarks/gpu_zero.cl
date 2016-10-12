// GPU memory 0
// (c) EB Sep 2009

#if (WORD_SIZE == 64)
typedef long Word;
#elif (WORD_SIZE == 32)
typedef int Word;
#elif (WORD_SIZE == 16)
typedef short Word;
#elif (WORD_SIZE == 128)
typedef float4 Word;
#endif

__kernel void zero(__global Word * a)
{
  int i = get_global_id(0);
  a[i] = 0.0f;
}
