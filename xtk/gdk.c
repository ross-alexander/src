#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <cairo.h>
#include <gdk/gdk.h>

#include "common.h"

/* ----------------------------------------------------------------------
--
-- xtk_create
--
---------------------------------------------------------------------- */

xtk_info_t info = {.id = "gdk"};

const char* id(void)
{
  return "gdk";
}

xtk_t* create(int w, int h, char *path)
{
  xtk_t *xtk = calloc(1, sizeof(xtk_t));
  xtk->width = w;
  xtk->height = h;

  if (!path)
    return xtk;

  GError *error = nullptr;
  if(!gdk_pixbuf_get_file_info(path, &w, &h))
    {
      fprintf(stderr, "GdkPixbuf failed.\n");
      return xtk;
    }
  xtk->title = strdup(path);
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, &error);
  if (pixbuf == nullptr)
    {
      fprintf(stderr, "gdk_pixbuf_new_from_file(%s) failed: %s\n", path, error->message);
      return xtk;
    }
      
  uint32_t pb_width = gdk_pixbuf_get_width(pixbuf);
  uint32_t pb_height = gdk_pixbuf_get_height(pixbuf);
  uint32_t pb_alpha = gdk_pixbuf_get_has_alpha(pixbuf);
  
  printf("Pixbuf (%d x %d alpha %d) from %s\n", pb_width, pb_height, pb_alpha, path);
      
  cairo_format_t format = pb_alpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24;
  cairo_surface_t *surface = cairo_image_surface_create(format, pb_width, pb_height);

  xtk->width = pb_width;
  xtk->height = pb_height;

  cairo_t *cr = cairo_create(surface);
  gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
  cairo_paint(cr);
  cairo_destroy(cr);
  g_object_unref(pixbuf);
  xtk->source = surface;

  return xtk;
}
