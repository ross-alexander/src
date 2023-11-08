#include <stdio.h>

#include <cairo-svg.h>
#include "common.h"

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  bio v(argc, argv);

  cairo_surface_t *r = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, nullptr);
  cairo_t *cr = cairo_create(r);
  v.cairo(cr);
  cairo_destroy(cr);

  cairo_rectangle_t extents;
  cairo_recording_surface_ink_extents(r, &extents.x, &extents.y, &extents.width, &extents.height);

  printf("%02f %02f %02f %02f\n", extents.x, extents.y, extents.width, extents.height);
  
  cairo_surface_t *s = cairo_svg_surface_create("bio.svg", v.width, v.height);
  cr = cairo_create(s);
  cairo_set_source_surface (cr, r, 0, 0); // -extents.x, -extents.y);
  cairo_set_source_surface(cr, r, 0, 0);
  cairo_paint(cr);
  cairo_destroy(cr);
  cairo_surface_destroy(s);
}
