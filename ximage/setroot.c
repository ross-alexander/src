#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <cairo/cairo-xcb.h>

#include <gtk/gtk.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

/* ----------------------------------------------------------------------
--
-- cairo_band
--
---------------------------------------------------------------------- */

void cairo_band(cairo_surface_t* surface, int width, int height)
{
  cairo_t *cr = cairo_create(surface);

  cairo_pattern_t *p = cairo_pattern_create_linear(0, 0, width, height);
  cairo_pattern_add_color_stop_rgb(p, 0.0, 0, 0, 1);
  cairo_pattern_add_color_stop_rgb(p, 1.0, 0, 1, 0);

  cairo_set_source(cr, p);
  cairo_new_path(cr);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_fill(cr);
  cairo_destroy(cr);
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  int rootx, rooty, rootwidth, rootheight, rootbw, rootdepth;

  gtk_init(&argc, &argv);

  int i, screenNum;
  xcb_connection_t *connection = xcb_connect (NULL, &screenNum);
    if (connection == NULL)
    {
      fprintf(stderr, "%s:  unable to open display\n", argv[0]);
      exit(1);
    }

  const xcb_setup_t *setup = xcb_get_setup(connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  
  // we want the screen at index screenNum of the iterator
  for (i = 0; i < screenNum; ++i)
    {
      xcb_screen_next(&iter);
    }

  xcb_screen_t *screen = iter.data;
  xcb_visualtype_t *visual = xcb_aux_find_visual_by_id(screen, screen->root_visual);
  assert(visual);

  if (screen->root_depth != 24 || visual->_class != XCB_VISUAL_CLASS_TRUE_COLOR)
    {
      printf("Only 24 bit TrueColor visuals for now\n");
      exit(1);
    }

  int width = screen->width_in_pixels;
  int height = screen->height_in_pixels;
  int depth = screen->root_depth;
  xcb_drawable_t root = screen->root;
  xcb_pixmap_t pm = xcb_generate_id(connection);
  xcb_create_pixmap_checked(connection, depth, pm, root, width, height);
  xcb_flush(connection);

  cairo_surface_t *surface = cairo_xcb_surface_create(connection, pm, visual, width, height);
  assert(surface);

  printf("Depth = %d\nWidth=%d\nHeight=%d\n", depth, width, height);

  if (argc > 1)
    {
      GError *error = NULL;
      printf("Loading %s\n", argv[1]);
      GdkPixbuf *px = gdk_pixbuf_new_from_file(argv[1], &error);
      assert(px != NULL);
      cairo_t *cr = cairo_create(surface);

      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_paint(cr);
      int iwidth = gdk_pixbuf_get_width(px);
      int iheight = gdk_pixbuf_get_height(px);
      gdk_cairo_set_source_pixbuf(cr, px, (width-iwidth)/2, (height-iheight)/2);
      cairo_paint(cr);
      cairo_destroy(cr);
    }
  else
    {
      cairo_band(surface, width, height);
    }
  cairo_surface_destroy(surface);

  uint32_t mask = XCB_CW_BACK_PIXMAP;
  uint32_t value_list = pm;

  /* --------------------
     Set background to pixmap and clear window
     -------------------- */

  xcb_change_window_attributes(connection, root, mask, &value_list);
  xcb_free_pixmap(connection, pm);
  xcb_aux_clear_window(connection, root);
  xcb_flush(connection);
  xcb_disconnect(connection);
  return 0;
}
