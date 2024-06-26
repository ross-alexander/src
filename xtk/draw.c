#include <assert.h>

#include <X11/Xlib.h>
#include <cairo/cairo.h>
#include <gdk/gdk.h>
#include "common.h"

/* ----------------------------------------------------------------------
--
-- xtk_cairo_draw
--
---------------------------------------------------------------------- */

void xtk_draw_cairo(xtk_t *xtk, cairo_t *cr)
{
  if (xtk->source)
    {
      double width = cairo_image_surface_get_width(xtk->source);
      double height = cairo_image_surface_get_height(xtk->source);

      printf("xtk_draw_cairo: src: %d × %d  dst:  %d × %d\n", (int)width, (int)height, xtk->width, xtk->height);

      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
      cairo_rectangle(cr, 0, 0, xtk->width, xtk->height);
      cairo_fill(cr);

      cairo_translate(cr, (xtk->width - width)/2, (xtk->height - height)/2);
      
      cairo_set_source_surface(cr, xtk->source, 0, 0);
      cairo_rectangle(cr, 0, 0, width, height);
      cairo_fill(cr);
    }
  else
    {
      cairo_pattern_t *p = cairo_pattern_create_linear(0, 0, xtk->width, xtk->height);

      /* offset, red, green, blue, alpha */
      cairo_pattern_add_color_stop_rgba(p, 0, 1, 0, 0, 0.1);
      cairo_pattern_add_color_stop_rgba(p, 1, 0, 1, 0, 0.9);
      
      cairo_set_source(cr, p);
      cairo_new_path(cr);
      cairo_rectangle(cr, 0, 0, xtk->width, xtk->height);
      cairo_fill(cr);

      cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
      cairo_move_to(cr, 100, 100);
      cairo_select_font_face(cr, "Helvetica", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size(cr, 20);
      cairo_show_text(cr, "helvetica test");
      
      cairo_move_to(cr, 100, 160);
      cairo_select_font_face(cr, "Corbel", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size(cr, 20);
      cairo_show_text(cr, "corbel test");
    }
  cairo_show_page(cr);
}

/* ----------------------------------------------------------------------
--
-- xtk_cairo_draw_surface
--
---------------------------------------------------------------------- */

void xtk_draw_surface(xtk_t *xtk, cairo_surface_t *target)
{
  cairo_t *cr = cairo_create(target);
  xtk_draw_cairo(xtk, cr);
  cairo_destroy(cr);
}
