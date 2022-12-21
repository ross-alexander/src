#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <glib.h>

#include "common.h"

int dvd_cli(int argc, char* argv[], const char *device)
{
  struct dvd_t* info = lsdvd_read_dvd(device);
  assert(info->discinfo.disc_title);
  char *buf = readline("> ");
}
