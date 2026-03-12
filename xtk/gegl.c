#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <gegl.h>
#include <cairo.h>

#include "common.h"

xtk_info_t info = {.id = "gegl"};

const char* id(void)
{
  return "gegl";
}

void init(gint *argc, char ***argv)
{
  gegl_init(argc, argv);
}

xtk_t* create(int w, int h, char *path)
{
  xtk_t *xtk = calloc(1, sizeof(xtk_t));
  xtk->width = w;
  xtk->height = h;

  GeglNode *load;
  GeglRectangle size;
  cairo_surface_t *image;

  load = gegl_node_new();
  
  gegl_node_set(load,
		"operation", "gegl:load",
		"path", path,
		NULL);
  size = gegl_node_get_bounding_box(load);

  image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
				     size.width,
				     size.height);
  cairo_surface_flush(image);
  
  gegl_node_blit (load,
		  1.0 /*scale*/,
		  &size,
		  babl_format ("cairo-ARGB32"),
		  cairo_image_surface_get_data (image),
		  GEGL_AUTO_ROWSTRIDE,
		  GEGL_BLIT_DEFAULT);

  cairo_surface_mark_dirty(image);

  xtk->width = size.width;
  xtk->height = size.height;
  xtk->source = image;
  xtk->title = strdup(path);
  return xtk;
}
