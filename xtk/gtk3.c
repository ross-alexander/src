#include <assert.h>
#include <gtk/gtk.h>
#include "common.h"

/* ----------------------------------------------------------------------
--
-- GTK3 module
--
-- 2018.03.29
--   Updated for GTK+-3.22
--
---------------------------------------------------------------------- */

typedef struct xtk_gtk_t {
  GtkWidget *window, *drawing;
  xtk_t *xtk;
} xtk_gtk_t;


static gboolean do_gtk3_draw(GtkWidget *widget, cairo_t *cr, void *client_data)
{
  xtk_gtk_t *xtk = (xtk_gtk_t*)client_data;

  guint width = gtk_widget_get_allocated_width(widget);
  guint height = gtk_widget_get_allocated_height(widget);

  xtk->xtk->width = width;
  xtk->xtk->height = height;
  
  xtk_draw_cairo(xtk->xtk, cr);
  return 0;
}

/* ----------------------------------------------------------------------
--
-- do_gtk
--
---------------------------------------------------------------------- */

  static const char *css =
  ".clear { "
  "  background: none; "
  "  border: 1px solid black; "
  "}";

int do_xtk(int argc, char *argv[], unsigned int nwin, xtk_t **xtk)
{
  gtk_init (&argc, &argv);
  
  GdkScreen *screen = gdk_screen_get_default();
  GdkVisual *vis = gdk_screen_get_rgba_visual(screen);
  
  xtk_gtk_t gtk[nwin];

  /* --------------------
     Create window
     -------------------- */

  for (int i = 0; i < nwin; i++)
    {
      
      xtk_gtk_t *t = gtk + i;
      t->xtk = xtk[i];

      t->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title(GTK_WINDOW(t->window), xtk[i]->title);
      gtk_widget_set_visual(t->window, vis);

      GtkCssProvider *provider = gtk_css_provider_new ();
      gtk_css_provider_load_from_data (provider, css, -1, NULL);
      gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
						 GTK_STYLE_PROVIDER (provider),
						 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);


  /* --------------------
     Create drawing area
     -------------------- */

      t->drawing = gtk_drawing_area_new();
      gtk_container_add(GTK_CONTAINER(t->window), t->drawing);

      g_signal_connect(G_OBJECT(t->window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
      g_signal_connect(G_OBJECT(t->drawing), "draw", G_CALLBACK(do_gtk3_draw), t);

      gtk_widget_set_size_request(GTK_WIDGET(t->drawing), t->xtk->width, t->xtk->height);
      gtk_style_context_add_class(gtk_widget_get_style_context(t->window), "clear");      
      gtk_widget_show_all(t->window);
    }
  gtk_main();
  return 1;
}

const char *id() { return "gtk3"; }
