#include <stdio.h>
#include <gbm.h>

int main()
{
  printf("RGB888: %d\n", GBM_FORMAT_RGB888);
  printf("XRGB888: %d\n", GBM_FORMAT_XRGB8888);
  printf("ARGB888: %d\n", GBM_FORMAT_ARGB8888);
  printf("BO_XRGB888: %d\n", GBM_BO_FORMAT_XRGB8888);
  printf("BO_ARGB888: %d\n", GBM_BO_FORMAT_ARGB8888);
  return(0);
}
