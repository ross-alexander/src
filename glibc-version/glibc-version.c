#include <stdio.h>
#include <gnu/libc-version.h>

int main()
{
  printf("version: %s\n", gnu_get_libc_version());
  printf("release: %s\n", gnu_get_libc_release());
  return 0;
}

