#include <assert.h>

#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include "common.h"

typedef struct xtk_gtk_t {
  GtkWidget *window, *drawing;
  GdkPixmap *bg;
  xtk_t *xtk;
} xtk_gtk_t;


static gboolean do_gtk2_configure(GtkWidget *win, GdkEventConfigure *event, void *client_data)
{
  xtk_gtk_t *xtk = (xtk_gtk_t*)client_data;
  xtk->xtk->width = event->width;
  xtk->xtk->height = event->height;
  return 0;
}

static gboolean do_gtk2_realise(GtkWidget *win, GdkEventConfigure *event, void *client_data)
{
  return 0;
}

static gboolean do_gtk2_expose(GtkWidget *win, GdkEventExpose *event, void *client_data)
{
  xtk_gtk_t *xtk = (xtk_gtk_t*)client_data;
  cairo_t *cr = gdk_cairo_create(win->window);
  xtk_draw_cairo(xtk->xtk, cr);
  cairo_destroy(cr);
  return 0;
}

/* ----------------------------------------------------------------------
--
-- do_gtk
--
---------------------------------------------------------------------- */
int do_xtk(int argc, char *argv[], unsigned int nwin, xtk_t **xtk)
{
  gtk_init (&argc, &argv);

  GdkScreen* screen = gdk_screen_get_default();
  GdkColormap *cmap = gdk_screen_get_rgba_colormap(screen);
  assert(cmap);

  xtk_gtk_t gtk[nwin];

  /* --------------------
     Create window
     -------------------- */

  for (int i = 0; i < nwin; i++)
    {
      
      xtk_gtk_t *t = gtk + i;
      t->xtk = xtk[i];

      t->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      gtk_widget_set_colormap(t->window, cmap);
      t->bg = gdk_pixmap_new(NULL, t->xtk->width, t->xtk->height, 32);
      gdk_drawable_set_colormap(t->bg, cmap);

      cairo_t *cr = gdk_cairo_create(t->bg);
      cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
      cairo_new_path(cr);
      cairo_set_source_rgba(cr, 0, 0, 0, 0);
      cairo_rectangle(cr, 0, 0, t->xtk->width, t->xtk->height);
      cairo_fill(cr);

  /* --------------------
     Create drawing area
     -------------------- */

      t->drawing = gtk_drawing_area_new();
      gtk_container_add(GTK_CONTAINER(t->window), t->drawing);

      gtk_widget_add_events(t->drawing, GDK_CONFIGURE);
      gtk_widget_add_events(t->drawing, GDK_EXPOSE);


      g_signal_connect(G_OBJECT(t->window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
      g_signal_connect(G_OBJECT(t->window), "realize", G_CALLBACK(do_gtk2_realise), t);
      g_signal_connect(G_OBJECT(t->drawing), "expose-event", G_CALLBACK(do_gtk2_expose), t);
      g_signal_connect(G_OBJECT(t->drawing), "realize", G_CALLBACK(do_gtk2_realise), t);
      g_signal_connect(G_OBJECT(t->drawing), "configure-event", G_CALLBACK(do_gtk2_configure), t);

      gtk_widget_set_size_request(GTK_WIDGET(t->drawing), t->xtk->width, t->xtk->height);
      
      //  gtk_window_set_decorated(window, FALSE);
      
      /*
	It is necessary to set the background to something.  If you just
	set the background to NULL using gdk_window_set_back_pixmap then
	the style system kicks in the fills the background, rather than
	leaving it blank.
	
	It can be done by setting the BG using gdk or using a gtk style.
  */

  /*
    GtkStyle *style = gtk_style_new();
    style->bg_pixmap[0] = bg;
    gtk_widget_set_style(drawing, style);
  */

      if (!GTK_WIDGET_VISIBLE(t->drawing))
	{
	  gtk_widget_realize(t->drawing);
	  gdk_window_set_back_pixmap(t->drawing->window, t->bg, FALSE);
	  //      gtk_widget_show(drawing);
	}
      gtk_widget_show_all(t->window);
    }
  gtk_main();
  return 1;
}

const char *id() { return "gtk2"; }

