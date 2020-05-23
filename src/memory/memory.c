#include "memory.h"
void memset(void* ptr, char v, int len)
{
    int i = 0;
    for (i = 0; i < len; i++)
    {
        ((char*)ptr)[i] = v;
    }
}

int memcmp (void* s1, void* s2, int count)
{
  char* c1 = s1;
  char* c2 = s2;
  while (count-- > 0)
    {
      if (*c1++ != *c2++)
	  return c1[-1] < c2[-1] ? -1 : 1;
    }
  return 0;
}


void* memcpy (void *dest,  void *src, int len)
{
  char *d = dest;
  char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}
