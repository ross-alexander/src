/* ----------------------------------------------------------------------
--
-- gtk4 display
--
-- Based on https://docs.gtk.org/gtk4/getting_started.html
--
-- 2022-02-19
--
---------------------------------------------------------------------- */

#include <gtk/gtk.h>

#include "common.h"

typedef struct xtk_gtk4_t {
  GtkWidget *window, *drawing;
  //  cairo_surface_t *surface;
  xtk_t *xtk;
} xtk_gtk4_t;

/* ----------------------------------------------------------------------
--
-- resize_cb
--
---------------------------------------------------------------------- */

static void resize_cb (GtkWidget *widget,
           int        width,
           int        height,
           gpointer   data)
{
  xtk_gtk4_t *gtk4 = (xtk_gtk4_t*)data;

  printf("gtk4: resize to %d × %d\n", width, height);
  gtk4->xtk->width = width;
  gtk4->xtk->height = height;

#ifdef RESIZE
  
  if (gtk4->surface)
    {
      cairo_surface_destroy (gtk4->surface);
      gtk4->surface = NULL;
    }

  
  GtkNative *native = gtk_widget_get_native(widget);
  
  if (native)
    {
      gtk4->surface = gdk_surface_create_similar_surface(gtk_native_get_surface(native), CAIRO_CONTENT_COLOR, gtk_widget_get_width(widget), gtk_widget_get_height(widget));
      
      /* --------------------
	 Center image
	 -------------------- */
      
      int x_off = (gtk4->xtk->width - width) / 2;
      int y_off = (gtk4->xtk->height - height) / 2;

      cairo_t *cr = cairo_create(gtk4->surface);
      cairo_translate(cr, -x_off, -y_off);
      cairo_set_source_surface(cr, gtk4->xtk->source, 0, 0);
      cairo_paint(cr);
      cairo_destroy(cr);
    }

#endif
}

/* ----------------------------------------------------------------------
--
-- draw_cb
--
---------------------------------------------------------------------- */

static void draw_cb (GtkDrawingArea *drawing_area,
		     cairo_t        *cr,
		     int             width,
		     int             height,
		     gpointer        data)
{
  xtk_gtk4_t *gtk4 = (xtk_gtk4_t*)data;
  printf("GTK4: draw_cb\n");
  xtk_draw_cairo(gtk4->xtk, cr);
}


static void activate (GtkApplication *app, gpointer user_data)
{
  xtk_t **xtk_list = (xtk_t**)user_data;

  for (int i = 0; xtk_list[i]; i++)
    {
      xtk_t *xtk = xtk_list[i];
      xtk_gtk4_t *gtk4 = calloc(sizeof(xtk_gtk4_t), 1);
      gtk4->xtk  = xtk;

      gtk4->window = gtk_application_window_new (app);

      if (xtk->title)
	gtk_window_set_title (GTK_WINDOW (gtk4->window), xtk->title);
      else
	gtk_window_set_title (GTK_WINDOW (gtk4->window), "Window");

      // gtk_window_set_default_size (GTK_WINDOW (gtk4->window), xtk->width, xtk->height);

      gtk_widget_set_visible(gtk4->window, 1);
      //      gtk_widget_show (gtk4->window);

      // Get monitor size to allow drawing to be clipped
      
      GdkSurface *s = gtk_native_get_surface(GTK_NATIVE(gtk4->window));
      GdkDisplay *d = gdk_surface_get_display(s);
      GdkMonitor *m = gdk_display_get_monitor_at_surface(d, s);

      GdkRectangle geometry;
      gdk_monitor_get_geometry(m, &geometry);

      // Don't clip to monitor currently
      
      //      printf("gtk4: Monitor [%d × %d]\n", geometry.width, geometry.height);
      
      gtk4->drawing = gtk_drawing_area_new();
      gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(gtk4->drawing), xtk->width);
      gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(gtk4->drawing), xtk->height);
      
      //      gtk_widget_set_size_request (gtk4->drawing, xtk->width, xtk->height);
      gtk_window_set_child(GTK_WINDOW(gtk4->window), gtk4->drawing);

      gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA (gtk4->drawing), draw_cb, gtk4, NULL);
      g_signal_connect_after(gtk4->drawing, "resize", G_CALLBACK (resize_cb), gtk4);
    }
}

int do_xtk(int argc, char *argv[], unsigned int nwin, xtk_t **xtk)
{
  int status;

  GtkApplication *app;
  app = gtk_application_new ("net.hepazulian.xtk", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), xtk);
  status = g_application_run (G_APPLICATION(app), 0, argv);
  g_object_unref(app);
  return status;
}

const char *id() { return "gtk4"; }
