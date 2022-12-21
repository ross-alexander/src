#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <gegl.h>
#include <cairo/cairo.h>


#include "common.h"

const char* id(void)
{
  return "gegl";
}

void init(gint *argc, char ***argv)
{
  printf("gegl-init\n");
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

  load = gegl_node_new ();
  
  gegl_node_set (load,
		 "operation", "gegl:load",
		 "path", path,
		 NULL);

  size = gegl_node_get_bounding_box (load);

  image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
				      size.width,
				      size.height);
  cairo_surface_flush(image);

  /*
  const Babl *format = babl_format_new (babl_model ("R'aG'aB'aA"),
					babl_type ("u8"),
					babl_component ("B'a"),
					babl_component ("G'a"),
					babl_component ("R'a"),
					babl_component ("A"),
					NULL);
  */
  
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
  xtk->surface = image;
  return xtk;
}
