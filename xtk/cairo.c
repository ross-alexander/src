#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cairo/cairo.h>

#include "common.h"

/* ----------------------------------------------------------------------
--
-- xtk_create
--
---------------------------------------------------------------------- */

xtk_t* create(int w, int h, char *path)
{
  xtk_t *xtk = calloc(1, sizeof(xtk_t));
  xtk->width = w;
  xtk->height = h;
  if (path)
    {
      cairo_surface_t *image = cairo_image_surface_create_from_png(path);
      if (!cairo_surface_status(image))
	{
	  xtk->width = cairo_image_surface_get_width(image);
	  xtk->height = cairo_image_surface_get_height(image);
	  xtk->source = image;
	  xtk->title = strdup(path);
	  printf("cairo: %s [%d × %d]\n", path, xtk->width, xtk->height);
	}
      else
	{
	  printf("Failed to get image %s\n", path);
	}
    }
  return xtk;
}

const char* id(void)
{
  return "cairo";
}
