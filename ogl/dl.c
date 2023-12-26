/* ----------------------------------------------------------------------
--
-- 2023-12-26: Add comment
--
---------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include <dlfcn.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <gtk/gtk.h>
#include <gtkgl/gtkglarea.h>
#include <GL/glut.h>

#include "dl.h"

/* ----------------------------------------------------------------------
--
-- subwin
--
---------------------------------------------------------------------- */

struct subwin {
  struct id *id;
  GtkWidget *win;
  int open;
  char *path;
  void *handle;
  int flags;
  void (*init)(void);
  void (*reshape)(int, int);
  void (*display)(void);
  int (*mouse)(int, int, int);
  int (*keyboard)(unsigned char, int, int);
};

enum {
  COLUMN_DESC,
  COLUMN_SUBWIN,
  N_COLUMNS,
};

struct tree_iter {
  GtkListStore *store;
  GtkTreeIter iter;
};

/* ----------------------------------------------------------------------
--
-- glarea_draw
--
---------------------------------------------------------------------- */

gint glarea_draw (GtkWidget* widget, GdkEventExpose* event)
{
  if (event->count > 0)
    {
      return(TRUE);
    }
  
  //  g_print ("Expose Event\n");

  if (gtk_gl_area_make_current(GTK_GL_AREA(widget)))
    {
      struct subwin *sw = (struct subwin*)g_object_get_data(G_OBJECT(widget), "sw");
      if (sw->display) (*sw->display)();
      gtk_gl_area_swapbuffers (GTK_GL_AREA(widget));
    }
  return TRUE;
}

/* ----------------------------------------------------------------------
--
-- glarea_shape
--
---------------------------------------------------------------------- */

gint glarea_reshape (GtkWidget* widget, GdkEventConfigure* event)
{
  int w = widget->allocation.width;
  int h = widget->allocation.height;
  
  //  g_print ("Reshape Event\n");

  if (gtk_gl_area_make_current (GTK_GL_AREA(widget)))
    {
      struct subwin *sw = (struct subwin*)g_object_get_data(G_OBJECT(widget), "sw");
      if (sw->reshape) (*sw->reshape)(w, h);
    }
  return TRUE;
 }

/* ----------------------------------------------------------------------
--
-- glarea_init
--
---------------------------------------------------------------------- */

gint glarea_init (GtkWidget* widget)
{
  //  g_print("Realize Event\n");

  if (gtk_gl_area_make_current (GTK_GL_AREA(widget)))
    {
      struct subwin *sw = (struct subwin*)g_object_get_data(G_OBJECT(widget), "sw");
      if (sw->init) (*sw->init)();
    }
  return TRUE;
}

/* ----------------------------------------------------------------------
--
-- glarea_destroy
--
---------------------------------------------------------------------- */

gint glarea_destroy (GtkWidget* widget)
{
  //  g_print ("GTK GL Area Destroy Event\n");
  return TRUE;
}

/* ----------------------------------------------------------------------
--
-- glarea_button_press
--
---------------------------------------------------------------------- */

gint glarea_button_press(GtkWidget* widget, GdkEventButton* event)
{
  int x = event->x;
  int y = event->y;

  struct subwin *sw = (struct subwin*)g_object_get_data(G_OBJECT(widget), "sw");
  if (sw->mouse)
    {
      int redraw = (*sw->mouse)(event->button, x, y);
      if (redraw) gtk_widget_queue_draw(widget);
      return TRUE;
    }
  return FALSE;
}

/* ----------------------------------------------------------------------
--
-- glarea_key_press
--
---------------------------------------------------------------------- */

gint glarea_key_press(GtkWidget* widget, GdkEventKey* event)
{
      printf("Keypress\n");
  struct subwin *sw = (struct subwin*)g_object_get_data(G_OBJECT(widget), "sw");
  if (sw->keyboard)
    {
      printf("Keypress\n");
      (*sw->keyboard)(event->keyval, 0, 0);
      return TRUE;
    }
  return FALSE;
}

/* ----------------------------------------------------------------------
--
-- create_glarea
--
---------------------------------------------------------------------- */

GtkWidget* create_glarea(void)
{
  GtkWidget* glarea;
  
  int attrlist[] = {
    GDK_GL_RGBA,
    GDK_GL_DOUBLEBUFFER,
    GDK_GL_DEPTH_SIZE, 24,
    GDK_GL_ALPHA_SIZE, 8,
    GDK_GL_NONE
  };

  if ((glarea = gtk_gl_area_new(attrlist)) == NULL)
    {
      g_print("Error creating GtkGLArea!\n");
      return NULL;
    }

  gtk_widget_set_events(GTK_WIDGET(glarea),
                        GDK_EXPOSURE_MASK|
			GDK_KEY_PRESS_MASK|
                        GDK_BUTTON_PRESS_MASK|
			GDK_BUTTON_RELEASE_MASK|
			GDK_POINTER_MOTION_MASK|
                        GDK_POINTER_MOTION_HINT_MASK);

  gtk_signal_connect(GTK_OBJECT(glarea), "expose_event", GTK_SIGNAL_FUNC(glarea_draw), NULL);
  gtk_signal_connect(GTK_OBJECT(glarea), "configure_event", GTK_SIGNAL_FUNC(glarea_reshape), NULL);
  gtk_signal_connect(GTK_OBJECT(glarea), "realize", GTK_SIGNAL_FUNC(glarea_init), NULL);
  gtk_signal_connect(GTK_OBJECT(glarea), "destroy", GTK_SIGNAL_FUNC (glarea_destroy), NULL);
  gtk_signal_connect(GTK_OBJECT(glarea), "button_press_event", GTK_SIGNAL_FUNC(glarea_button_press), NULL);
  gtk_signal_connect(GTK_OBJECT(glarea), "key_press_event", GTK_SIGNAL_FUNC(glarea_key_press), NULL);
  gtk_widget_set_usize(GTK_WIDGET(glarea), 800, 800);
  return (glarea);
}

/* ----------------------------------------------------------------------
--
-- subwin_hide
--
---------------------------------------------------------------------- */
void subwin_hide(GtkWidget *button, void *data)
{
  GtkWidget *win = (GtkWidget*)data;
  gtk_widget_hide(win);
}

/* ----------------------------------------------------------------------
--
-- make_window
--
---------------------------------------------------------------------- */
int make_window(char *desc, struct subwin *sw, void* data)
{
  GdkScreen* screen = gdk_screen_get_default();
  GdkColormap *cmap = gdk_screen_get_rgba_colormap(screen);
  assert(cmap);

  GtkWidget* box_main = gtk_vbox_new (FALSE, 10);
  GtkWidget* glarea = create_glarea();

  assert(glarea != NULL);

  g_object_set_data(G_OBJECT(glarea), "sw", sw);

  GtkWidget* button_quit = gtk_button_new_with_label ("Hide");
  GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_colormap(window, cmap);
  gtk_window_set_title (GTK_WINDOW(window), "GtkGLArea Demo");

  gtk_quit_add_destroy (1, GTK_OBJECT(window));

  gtk_signal_connect (GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
  gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
  gtk_signal_connect (GTK_OBJECT(button_quit), "clicked", GTK_SIGNAL_FUNC(subwin_hide), window);

  gtk_container_set_border_width (GTK_CONTAINER(window), 10);

  gtk_box_pack_start (GTK_BOX(box_main), glarea,      FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX(box_main), button_quit, FALSE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER(window), box_main);

  gtk_widget_show (glarea);
  gtk_widget_show (button_quit);
  gtk_widget_show (box_main);
  sw->win = window;
  printf("%s window created.\n", desc);

  /* return 0 so traverse will continue */

  return 0;
}

int tree_add(char *desc, struct subwin *sw, struct tree_iter *iter)
{
  gtk_list_store_append (iter->store, &iter->iter);
  gtk_list_store_set (iter->store, &iter->iter, COLUMN_DESC, desc, -1);
  gtk_list_store_set (iter->store, &iter->iter, COLUMN_SUBWIN, sw, -1);
  return 0;
}

/* ----------------------------------------------------------------------
--
-- mod_load
--
---------------------------------------------------------------------- */
int mod_load(struct subwin *sw)
{
  char *error;
  if (sw->handle)
    {
      printf("Reloading %s %p ", sw->id->desc, sw->handle);
      int result = dlclose(sw->handle);
      printf("%d\n", result);
    }
  if ((sw->handle = dlopen(sw->path, RTLD_LAZY|RTLD_LOCAL)) == 0)
    {
      fprintf(stderr, "Error in dlopen: %s\n", dlerror());
      return 0;
    }
  dlerror();
  sw->id = (struct id*)dlsym(sw->handle, "id");
  printf("%s %p\n", sw->id->desc, sw->handle);

  error = dlerror();
  if (error)
    {
      fprintf(stdout, "symbol 'id' missing.\n");
      dlclose(sw->handle);
      return 0;
    }
  else
    {
      sw->init = dlsym(sw->handle, "init");
      sw->reshape = dlsym(sw->handle, "reshape");
      sw->display = dlsym(sw->handle, "display");
      sw->mouse = dlsym(sw->handle, "mouse");
      sw->keyboard = dlsym(sw->handle, "keyboard");
    }
  return 1;
}

/* ----------------------------------------------------------------------
--
-- dl_insert
--
---------------------------------------------------------------------- */

int dl_insert(GTree *modtree, char *path)
{
  struct subwin *sw = calloc(sizeof(struct subwin), 1);
  sw->path = strdup(path);
  if (mod_load(sw))
    {
      g_tree_insert(modtree, sw->id->desc, sw);
      return 1;
    }
  else
    {
      free(sw->path);
      free(sw);
    }
  return 0;
}

/* ----------------------------------------------------------------------
--
-- scandir
--
---------------------------------------------------------------------- */
  GTree* dl_scandir(GTree *modtree, char *dir)
{
  DIR *dirp;
  struct dirent *dirent;

  modtree = modtree != 0 ? modtree : g_tree_new((GCompareFunc)strcmp);
  dirp = opendir(dir);
  while ((dirent = readdir(dirp)) != NULL)
    {
      char *n = dirent->d_name;
      int l = strlen(n);
      if ((l > 3) && (strcmp(n + l - 3, ".so") == 0))
	{
	  char *path = malloc(strlen(dir) + l + 2);
	  snprintf(path, strlen(dir) + l + 2, "%s/%s", dir, n);
	  dl_insert(modtree, path);
	}
    }
  closedir(dirp);
  return modtree;
}

int selection_changed (GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, gpointer data)
{ 
  GtkTreeIter iter;
  gchar *desc;
  struct subwin *sw;
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_DESC, &desc, -1);
  gtk_tree_model_get (model, &iter, COLUMN_SUBWIN, &sw, -1);

  if (!path_currently_selected)
    gtk_window_present((GtkWindow*)sw->win);

  //  printf("Selected %d %s\n", path_currently_selected, desc);
  return 0;
}

/* ----------------------------------------------------------------------
--
-- dl_reload
--
---------------------------------------------------------------------- */
int dl_reload_recc(char *desc, struct subwin *sw, void* data)
{
  mod_load(sw);
  return 0;
}

void dl_reload(GtkWidget *widget, GTree *modtree)
{
  g_tree_foreach(modtree, (GTraverseFunc)dl_reload_recc, NULL);
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main (int argc, char** argv)
{
  char *dir = ".";
  gtk_init (&argc, &argv);
  glutInit(&argc, argv);
  GTree *modtree = g_tree_new((GCompareFunc)strcmp);

  /* If no arguments scan directory */
  
  if (argc < 1)
    {
      dl_scandir(modtree, dir);
    }
  else
    {
      if (dl_insert(modtree, argv[1]) == 0)
	{
	  fprintf(stderr, "%s: failed to load %s\n", argv[0], argv[1]);
	  exit(1);
	}
    }

  /* Make window for each subwindow */
  
  g_tree_foreach(modtree, (GTraverseFunc)make_window, NULL);
  
  GtkListStore *list_store;

  struct tree_iter iter;
  list_store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
  iter.store = list_store;

  g_tree_foreach(modtree, (GTraverseFunc)tree_add, &iter);

  /* --------------------
     Do main window
     -------------------- */

  GtkWidget *window;
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (gtk_main_quit), NULL);

  /* --------------------
     Create tree
     -------------------- */

  GtkWidget *tree;
  tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree), COLUMN_SUBWIN);
  g_object_unref(G_OBJECT(list_store));

  /* --------------------
     Add columns
     -------------------- */

  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeView *treeview = GTK_TREE_VIEW(tree);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Host",
                                                     renderer,
                                                     "text", COLUMN_DESC,
                                                     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_DESC);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);
  
  /* --------------------
     Sort out selection
     -------------------- */

  GtkTreeSelection *selection;
  struct entry *ent = NULL;
  GtkTreePath *path;
 
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
  gtk_tree_selection_unselect_all(selection);
  gtk_tree_selection_set_select_function(selection, selection_changed, (void*)&ent, NULL);


  /* --------------------
     Create vbox for packing
     -------------------- */
  
  GtkWidget *vbox = gtk_vbox_new(0, 2);

  /* --------------------
     Create horizontal button box
     -------------------- */

  GtkWidget *bbox = gtk_hbutton_box_new();

  /* --------------------
     Create buttons
     -------------------- */

  GtkWidget *gobutton = gtk_button_new_with_label("Reload");
  g_signal_connect (G_OBJECT (gobutton), "pressed", G_CALLBACK (dl_reload), modtree);

  GtkWidget *qbutton = gtk_button_new_with_label("Quit");
  g_signal_connect (G_OBJECT (qbutton), "pressed", G_CALLBACK (gtk_main_quit), NULL);

  /* --------------------
     Pack everything
     -------------------- */

  gtk_container_set_border_width (GTK_CONTAINER (window), 5);
  gtk_container_add(GTK_CONTAINER(bbox), gobutton);
  gtk_container_add(GTK_CONTAINER(bbox), qbutton);

  gtk_box_pack_start(GTK_BOX(vbox), tree, 1, 1, 0);
  gtk_box_pack_end(GTK_BOX(vbox), bbox, 0, 0, 0);

  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_widget_show_all (window);
  gtk_main();
  return (0);
}
