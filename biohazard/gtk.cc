#include <math.h>
#include <gtk/gtk.h>

#include "common.h"

double *theta;

/* ----------------------------------------------------------------------
--
-- do_gtk_realise
--
---------------------------------------------------------------------- */
gboolean do_gtk_draw(GtkWidget *win, GdkEventExpose *event, void *client_data)
{
  bio *v = (bio*)client_data;
  cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(win));
  v->cairo(cr);
  cairo_destroy(cr);
  (*theta)--;
  gtk_widget_queue_draw(win);

  return 0;
}

/* ----------------------------------------------------------------------
--
-- update_theta
--
---------------------------------------------------------------------- */
gboolean update_angle(gpointer data)
{
    GtkWidget *w = (GtkWidget*)data;
    (*theta)++;
    gtk_widget_queue_draw(w);
    return 1;

}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  bio v(argc, argv);
  gtk_init (&argc, &argv);
  GdkVisual *vis = gdk_visual_get_best_with_depth(32);

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_visual(window, vis);

  GtkWidget *drawing = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), drawing);

  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  //      g_signal_connect(G_OBJECT(t->window), "realize", G_CALLBACK(do_gtk_realise), t);
  g_signal_connect(G_OBJECT(drawing), "draw", G_CALLBACK(do_gtk_draw), &v);
  //      g_signal_connect(G_OBJECT(t->drawing), "realize", G_CALLBACK(do_gtk_realise), t);
  //      g_signal_connect(G_OBJECT(t->drawing), "configure-event", G_CALLBACK(do_gtk_configure), t);

  gtk_widget_set_size_request(GTK_WIDGET(drawing), v.width, v.height);

  gtk_window_set_decorated(GTK_WINDOW(window), 0);

  GdkRGBA bg = {0.0, 0.0, 0.0, 0.0};
  
  gtk_widget_override_background_color(window, (GtkStateFlags)0, &bg);
  gtk_widget_override_background_color(drawing, (GtkStateFlags)0, &bg);
  gtk_widget_show_all(window);

  theta = &v.rotation;
  //  g_timeout_add_seconds(1, update_angle, drawing);

  gtk_main();
}