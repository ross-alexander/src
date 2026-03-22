#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

char* int2binstr(uint32_t key, uint32_t len)
{
  char *res = calloc(sizeof(char), len + 2);
  for (int i = 0; i < len; i++)
    res[i] = key & (1 << (31 - i)) ? '1' : '0';
  return res;
}

int main()
{
  uint32_t b = 32;
  for (uint32_t i = 0; i <= b; i++)
    {
      uint32_t shift = 32 - i;
      //      uint32_t m =  ((uint32_t) 1 << i/2 << i-i/2) - 1;

      uint32_t m = (uint32_t)0xffffffff << shift/2 << (shift - shift/2); // & 0xFFFFFFFF;
      char *s = int2binstr(m, 32);
      printf("%02d -- %s\n", i, s);

      free(s);
    }
}
