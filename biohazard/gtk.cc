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

  v->width = width;
  v->height = height;
  v->cairo(cr);
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
  //  gtk_window_set_decorated(GTK_WINDOW(window), 0);

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
