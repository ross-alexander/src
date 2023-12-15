/* ----------------------------------------------------------------------
--
-- xtk
--
-- Simple window test with multiple backends via dynamic libraries
--
-- 2022-02-19
--
---------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdint.h>

#include <X11/Xutil.h>

#include <cairo/cairo.h>

#include "common.h"

/* ----------------------------------------------------------------------
--
-- load_dynlib
--
---------------------------------------------------------------------- */
void** load_dynlib(const char *dir)
{
  DIR *dirp = opendir(dir);
  struct dirent *dirent;
  int dlcnt = 0;

  /* could use fnmatch */

  while ((dirent = readdir(dirp)) != 0)
    {
      unsigned int l = strlen(dirent->d_name);
      if (strcmp(".so", dirent->d_name + l - 3) == 0)
	dlcnt++;
    }
  void **handle = calloc(sizeof(void*), dlcnt+1);
  dlcnt = 0;
  rewinddir(dirp);
  while ((dirent = readdir(dirp)) != 0)
    {
      unsigned int l = strlen(dirent->d_name);
      if (strcmp(".so", dirent->d_name + l - 3) == 0)
	{
	  int plen = strlen(dir) + l + 2;
	  char path[strlen(dir) + l + 2];
	  snprintf(path, plen, "%s/%s", dir, dirent->d_name);
	  dlerror();
	  handle[dlcnt] = dlopen(path, RTLD_LAZY|RTLD_LOCAL);
	  char *error = dlerror();
	  if (error)
	    {
	      printf("Failed to open %s: %s\n", dirent->d_name, error);
	    }
	  else
	    {
	      printf("Found dynamic library %s\n", path);
	      dlcnt++;
	    }
	}
    }
  printf("\n");
  handle[dlcnt] = 0;
  closedir(dirp);
  return handle;
}


/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  int width = 400, height = 400;
  int ch;
  char *display_lib = "xcb";
  char *render_lib = "cairo";
  unsigned int nwin;
  xtk_t* (*create)(uint32_t, uint32_t, const char*) = 0;
  
  xtk_t **xtk;

  while ((ch = getopt(argc, argv, "w:h:d:r:")) != EOF)
    switch(ch)
      {
      case 'w':
	width = strtol(optarg, 0, 10);
	break;
      case 'h':
	height = strtol(optarg, 0, 10);
	break;
      case 'd':
	display_lib = optarg;
	break;
      case 'r':
	render_lib = optarg;
	break;
      }

  /* --------------------
     Match render library
     -------------------- */
  
  void **render = load_dynlib("render");
  
  for (int j = 0; render[j]; j++)
    {
      if (!dlsym(render[j], "id"))
	continue;
      char* (*id)() = dlsym(render[j], "id");
      if (strcmp(render_lib, (*id)()) != 0)
	continue;
      create = dlsym(render[j], "create");

      printf("Found render library %s\n", render_lib);

      assert(create);
      void (*init)(int*, char***) = dlsym(render[j], "init");
      if (init) (*init)(&argc, &argv);
    }

  if (create == 0)
    {
      fprintf(stderr, "%s: Render %s not found\n", argv[0], render_lib);
      exit(1);
    }

  /* --------------------
     If cli options passed treat as file paths
     -------------------- */
  
  if (optind != argc)
    {
      nwin = argc - optind;
      printf("Creating %d windows\n", nwin);
      xtk = calloc(nwin + 1, sizeof(xtk_t*));
      for (int j = 0; j < nwin; j++)
	xtk[j] = (*create)(width, height, argv[optind + j]);
    }
  else
    {
      nwin = 1;
      xtk = calloc(nwin + 1, sizeof(xtk_t*));
      xtk[0] = (*create)(width, height, 0);
    }

  /* --------------------
     Use dynamic display library
     -------------------- */

  void **display = load_dynlib("display");
  for (int j = 0; display[j]; j++)
    {
      if (!dlsym(display[j], "id"))
	continue;
      char* (*id)() = dlsym(display[j], "id");
      if (strcmp(display_lib, (*id)()) != 0)
	continue;
      printf("Found display library %s\n", display_lib);
      int (*do_xtk)(int, char**, unsigned int, xtk_t**) = dlsym(display[j], "do_xtk");
      if (do_xtk)
	{
	  do_xtk(argc - optind, &argv[optind], nwin, xtk);
	  return 0;
	}
    }
  printf("Unknown interface %s\n", display_lib);

  return 0;
}
