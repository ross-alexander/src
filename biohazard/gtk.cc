#include <assert.h>
#include <math.h>
#include <gtk/gtk.h>

#include "common.h"

/* ----------------------------------------------------------------------
--
-- do_gtk_draw
--
---------------------------------------------------------------------- */
gboolean do_gtk_draw(GtkWidget *widget, cairo_t *cr, void *client_data)
{
  bio_t *v = (bio_t*)client_data;

  // "convert" the G*t*kWidget to G*d*kWindow (no, it's not a GtkWindow!)
  GdkWindow* window = gtk_widget_get_window(widget);

  guint width = gtk_widget_get_allocated_width (widget);
  guint height = gtk_widget_get_allocated_height (widget);

  //  fprintf(stdout, "draw(%d, %d)\n", width, height);
  
  v->width = width;
  v->height = height;

  cairo_region_t * cairoRegion = cairo_region_create();
  GdkDrawingContext * drawingContext;
  drawingContext = gdk_window_begin_draw_frame(window, cairoRegion);
  cairo_t * cr_win = gdk_drawing_context_get_cairo_context(drawingContext);

  v->cairo(cr_win);
  
  gdk_window_end_draw_frame(window, drawingContext);
  cairo_region_destroy(cairoRegion);
  v->theta -= 1.0;
  gtk_widget_queue_draw(widget);

  return 0;
}


/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  bio_t bio(argc, argv);
  gtk_init (&argc, &argv);

  GdkScreen *screen = gdk_screen_get_default();
  GdkVisual *visual = gdk_screen_get_rgba_visual(screen);

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_visual(window, visual);

  GtkWidget *drawing = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), drawing);
  gtk_widget_set_name(window, "drawing");
  
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(G_OBJECT(drawing), "draw", G_CALLBACK(do_gtk_draw), &bio);

  gtk_widget_set_size_request(GTK_WIDGET(drawing), bio.width, bio.height);
  gtk_window_set_decorated(GTK_WINDOW(window), 0);

  //  GdkRGBA bg = {0.0, 0.0, 0.0, 0.0};  
  //  gtk_widget_override_background_color(window, (GtkStateFlags)0, &bg);
  //  gtk_widget_override_background_color(drawing, (GtkStateFlags)0, &bg);

  GtkCssProvider *provider = gtk_css_provider_new ();
  assert(gtk_css_provider_load_from_path(provider, "bio.css", NULL));
  // gtk_css_provider_load_from_data (provider, css, -1, NULL);
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default(),
					     GTK_STYLE_PROVIDER(provider),
					     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  
  gtk_widget_show_all(window);

  //  g_timeout_add_seconds(1, update_angle, drawing);

  gtk_main();
}
