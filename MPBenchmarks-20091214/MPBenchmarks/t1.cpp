#include <stdio.h>

typedef long long int int64;

int main()
{
  const int64 Blog = 8;
  const int64 B = (int64)1<<Blog;

  for (int64 m=B;m<3*B ;m++)
    {
      int64 x = B-1+2*(m/B);
      if (x>m) continue;

      printf("Blog=%lld  m=%llX\n",Blog,m);
      // break;
    }
}
