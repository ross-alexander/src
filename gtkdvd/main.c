#include <stdlib.h>

int dvd_gtk(int argc, char *argv[], const char *device);
int dvd_cli(int argc, char *argv[], const char *device);

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  const char *device = "/dev/sr0";

  char *display = getenv("DISPLAY");
  if (display)
    dvd_gtk(argc, argv, device);
  else
    dvd_cli(argc, argv, device);
}
