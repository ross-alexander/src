#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cairo/cairo.h>

#include <gdk/gdk.h>

#include "common.h"

/* ----------------------------------------------------------------------
--
-- xtk_create
--
---------------------------------------------------------------------- */

const char* id(void)
{
  return "gdk";
}

xtk_t* create(int w, int h, char *path)
{
  xtk_t *xtk = calloc(1, sizeof(xtk_t));
  xtk->width = w;
  xtk->height = h;

  if (path)
    {
      GError *error = NULL;
      if(!gdk_pixbuf_get_file_info(path, &w, &h))
	{
	  fprintf(stderr, "GdkPixbuf failed.\n");
	}
      GdkPixbuf *pb = gdk_pixbuf_new_from_file(path, &error);
      assert(pb);

      xtk->title = strdup(path);
      
      uint32_t pb_width = gdk_pixbuf_get_width(pb);
      uint32_t pb_height = gdk_pixbuf_get_height(pb);
      uint32_t pb_alpha = gdk_pixbuf_get_has_alpha(pb);

      printf("Pixbuf (%d x %d alpha %d) from %s\n", pb_width, pb_height, pb_alpha, path);
      
      cairo_format_t format = gdk_pixbuf_get_has_alpha(pb) ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24;
      cairo_surface_t *surface = cairo_image_surface_create(format, gdk_pixbuf_get_width(pb), gdk_pixbuf_get_height(pb));

      xtk->width = pb_width;
      xtk->height = pb_height;
      
      cairo_t *cr = cairo_create(surface);
      gdk_cairo_set_source_pixbuf(cr, pb, 0, 0);
      cairo_paint(cr);
      cairo_destroy(cr);
      g_object_unref(pb);
      xtk->source = surface;
    }
  return xtk;
}
