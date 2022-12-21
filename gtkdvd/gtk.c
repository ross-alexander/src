#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

#include <gtk/gtk.h>
#include <dvdread/ifo_read.h>
#include <dvdread/nav_read.h>
#include <dvdnav/dvdnav.h>

#include "common.h"

/* --------------------
Main track table
-------------------- */

enum
  {
    TITLE_COLUMN,
    CHAPTER_COLUMN,
    TIME_COLUMN,
    FILE_COLUMN,
    PROGRESS_COLUMN,
    PTR_COLUMN,
    N_COLUMNS,
  };

/* --------------------
Track structure
-------------------- */

struct entry {
  gint title;
  gint chapters;
  gchar *time;
  gchar *file;
  gint progress;
  char *path;
  char *dpath;
};

/* --------------------
Primary system state record
-------------------- */

struct gtkdvd_info {
  char *device;
  struct dvd_info *dvd_info;
  struct entry *entries;
  GtkListStore *store, *dstore;
  GtkWidget *window, *dialog;
  GtkWidget *textbox;
  int ntracks;
};

GMutex* riplock;

/* ----------------------------------------------------------------------
--
-- gtk_rip
--
---------------------------------------------------------------------- */

gpointer gtk_rip_mt(void *data)
{
  struct gtkdvd_info *lsdvd = (struct gtkdvd_info*)data;
  struct entry *e = lsdvd->entries;
  char *device = lsdvd->dvd_info->discinfo.device;
  unsigned char *readdata = malloc(1024 * DVD_VIDEO_LB_LEN);
  for (int i = 0; e[i].title != -1; i++)
    if (e[i].file && strlen(e[i].file))
      {
	int title = e[i].title - 1;
	FILE *stream;
	if (stream = fopen64(e[i].file, "wb"))
	  {
	    struct entry *ent;
	    int n_chapters = lsdvd->dvd_info->titles[title].chapter_count_reported;
	    printf("Writing title %d (%d chapters) to %s from %s\n", e[i].title, n_chapters, e[i].file, device);
	    GtkTreeIter iter;
	    GtkTreeModel *tree = GTK_TREE_MODEL(lsdvd->dstore);
	    GtkTreePath *path = gtk_tree_path_new_from_string(e[i].dpath);
	    gtk_tree_model_get_iter(tree, &iter, path);
	    gtk_tree_model_get(tree, &iter, PTR_COLUMN, &ent, -1);
	    assert(ent == &e[i]);

	    for (int j = 1; j <= n_chapters; j++)
	      {
		tc_dvd_read(device, e[i].title, j, 1, readdata, 1, stream);
		gtk_list_store_set(GTK_LIST_STORE(lsdvd->dstore), &iter,
				   PROGRESS_COLUMN, (int)((double)j/(double)n_chapters * 100),
				   -1);
	      }
	    fclose(stream);
	    printf("Copy finished\n");
	  }
      }
  free(readdata);
  g_mutex_unlock(riplock);
  g_thread_exit(0);
}

int gtk_rip(GtkWidget *obj, void *data)
{
  struct gtkdvd_info *lsdvd = (struct gtkdvd_info*)data;
  GtkWidget *dialog =  gtk_dialog_new_with_buttons ("Rip progress",
						    GTK_WINDOW(lsdvd->window),
						    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						    GTK_STOCK_OK,
						    GTK_RESPONSE_ACCEPT,
						    GTK_STOCK_CANCEL,
						    GTK_RESPONSE_REJECT,
						    NULL);

  GtkListStore *store = gtk_list_store_new(N_COLUMNS, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_POINTER);
  for (int i = 0; i < lsdvd->dvd_info->title_count; i++) 
    {
      if (lsdvd->entries[i].file && strlen(lsdvd->entries[i].file))
	{
	  struct entry *ent = &lsdvd->entries[i];
      
      /* --------------------
	 User Iterator to dvd->store data
	 -------------------- */
      
	  GtkTreeIter iter;
	  gtk_list_store_append(store, &iter);
	  gtk_list_store_set(store, &iter,
			     TITLE_COLUMN, ent->title,
			     CHAPTER_COLUMN, ent->chapters,
			     TIME_COLUMN, ent->time,
			     FILE_COLUMN, ent->file,
			     PROGRESS_COLUMN, ent->progress,
			     PTR_COLUMN, ent,
			     -1);
	  ent->dpath = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(store), &iter);
	  printf("Add new entry at %s\n", ent->dpath);
	}
    }

  /* --------------------
     Create tree
     -------------------- */
  
  GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
  gtk_tree_view_set_rules_hint(GTK_TREE_VIEW (tree), TRUE);
  g_object_unref(G_OBJECT(store));

  /* --------------------
     Add columns
     -------------------- */

  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeView *treeview = GTK_TREE_VIEW(tree);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Title", renderer, "text", TITLE_COLUMN, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);


  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Chapters", renderer, "text", CHAPTER_COLUMN, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);


  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Time", renderer, "text", TIME_COLUMN, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);


  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("File", renderer, "text", FILE_COLUMN, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);

  renderer = gtk_cell_renderer_progress_new ();
  column = gtk_tree_view_column_new_with_attributes ("Progress", renderer, "value", PROGRESS_COLUMN, NULL);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 500);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);

  GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

  //  gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
  //  gtk_box_pack_start(GTK_BOX(vbox), tree, 1, 1, 0);

  GtkWidget *label =  gtk_label_new ("Ripping progress");

  gtk_container_add(GTK_CONTAINER(content), label);
  gtk_container_add(GTK_CONTAINER(content), tree);

  g_signal_connect_swapped (dialog,
			    "response",
			    G_CALLBACK (gtk_widget_destroy),
			    dialog);

  gtk_widget_show_all (dialog);
  lsdvd->dstore = store;

  if (g_mutex_trylock(riplock))
    {
      GError *error;
      GThread *thread = g_thread_create(gtk_rip_mt, data, 0, &error);
    }
  else
    {
      printf("RIP locked\n");
    }
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_main_quit();
}

/* ----------------------------------------------------------------------
--
-- entry_edited_callback
--
---------------------------------------------------------------------- */

void entry_edited_callback (GtkCellRendererText *cell,
			    gchar               *path_string,
			    gchar               *new_text,
			    gpointer             user_data)
{
  struct entry *ent;
  GtkTreeIter iter;
  GtkTreeModel *tree = GTK_TREE_MODEL(user_data);
  GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
  gtk_tree_model_get_iter (tree, &iter, path);
  gtk_tree_model_get (tree, &iter, PTR_COLUMN, &ent, -1);
  if (ent->file)
    free(ent->file);
  ent->file = strdup(new_text);
  gtk_list_store_set(GTK_LIST_STORE(user_data), &iter,
		     FILE_COLUMN, ent->file,
		     -1);
}

/* ----------------------------------------------------------------------
--
-- selection_changed
--
---------------------------------------------------------------------- */

int selection_changed (GtkTreeSelection *selection, GtkTreeModel *model,
GtkTreePath *path, gboolean path_currently_selected, gpointer data)
{
  GtkTreeIter iter;
  struct gtkdvd_info *dvd = (struct gtkdvd_info*)data;
  struct entry *e;

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, PTR_COLUMN, &e, -1);

  GString *s = dvd_gprint_title(dvd->dvd_info, e->title - 1);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dvd->textbox));
  gtk_text_buffer_set_text(buffer, s->str, -1);
  g_string_free(s, TRUE);
  return 1;
}

/* ----------------------------------------------------------------------
--
-- dvd_gtk
--
---------------------------------------------------------------------- */

int dvd_gtk(int argc, char *argv[], const char *device)
{
  g_thread_init(NULL);
  riplock = g_mutex_new();

  struct gtkdvd_info *dvd = calloc(1, sizeof(struct gtkdvd_info));
  dvd->device = strdup(device);
  dvd->dvd_info = lsdvd_read_dvd(device);

  struct dvd_info *dvd_info = dvd->dvd_info;

  /* --------------------
     call gtk_init to start with
     -------------------- */

  gtk_init(&argc, &argv);
  
  /* --------------------
     Create List Store
     -------------------- */
  
  dvd->store = gtk_list_store_new(N_COLUMNS, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_POINTER);

  struct entry *entries = dvd->entries = calloc(sizeof(struct entry), dvd_info->title_count + 1);
  entries[dvd_info->title_count].title = -1;

  for (int i = 0; i < dvd_info->title_count; i++) 
    {
      entries[i].title = i+1;
      entries[i].time = g_strdup_printf("%02d:%02d:%02d.%03d",
				dvd_info->titles[i].general.playback_time.hour,
				dvd_info->titles[i].general.playback_time.minute,
				dvd_info->titles[i].general.playback_time.second,
				dvd_info->titles[i].general.playback_time.usec);
      entries[i].chapters = dvd_info->titles[i].chapter_count_reported;
      entries[i].file = 0;
      struct entry *ent = &entries[i];

      /* --------------------
	 User Iterator to dvd->store data
	 -------------------- */
      
      GtkTreeIter iter;
      gtk_list_store_append(dvd->store, &iter);
      gtk_list_store_set(dvd->store, &iter,
			 TITLE_COLUMN, ent->title,
			 CHAPTER_COLUMN, ent->chapters,
			 TIME_COLUMN, ent->time,
			 FILE_COLUMN, ent->file,
			 PROGRESS_COLUMN, ent->progress,
			 PTR_COLUMN, ent,
			 -1);
      ent->path = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(dvd->store), &iter);
    }

  /* --------------------
     Do main window
     -------------------- */

  GtkWidget *window;
  dvd->window = window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (gtk_main_quit), NULL);

  /* --------------------
     Create tree
     -------------------- */

  GtkWidget *tree;
  tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (dvd->store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree), TITLE_COLUMN);
  g_object_unref(G_OBJECT(dvd->store));

  /* --------------------
     Add columns
     -------------------- */

  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeView *treeview = GTK_TREE_VIEW(tree);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes ("Title", renderer, "text", TITLE_COLUMN, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);


  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes ("Chapters", renderer, "text", CHAPTER_COLUMN, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes ("Time", renderer, "text", TIME_COLUMN, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);


  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("File", renderer, "text", FILE_COLUMN, NULL);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 500);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);
  g_object_set(renderer, "editable", TRUE, NULL);
  g_signal_connect(renderer, "edited", (GCallback)entry_edited_callback, dvd->store);

  /* --------------------
     Sort out selection
     -------------------- */

  GtkTreeSelection *selection;
  struct entry *ent = NULL;

  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
  gtk_tree_selection_unselect_all(selection);
  gtk_tree_selection_set_select_function(selection, selection_changed, (void*)dvd, NULL);

  /* --------------------
     Create textbuf and scrolled
     -------------------- */

  GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

  GtkWidget *scrolled = gtk_scrolled_window_new(0, 0);

  GtkWidget *textbox = dvd->textbox = gtk_text_view_new();
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dvd->textbox));
  
  gtk_text_buffer_set_text(buffer, dvd_info->discinfo.disc_title, -1);
  gtk_widget_set_size_request (textbox, -1, 400);

  gtk_container_add(GTK_CONTAINER(scrolled), textbox);

  /* --------------------
     Create buttons
     -------------------- */

  GtkWidget *gobutton = gtk_button_new_with_label("RIP");
  g_signal_connect (G_OBJECT (gobutton), "pressed", G_CALLBACK (gtk_rip), dvd);

  GtkWidget *qbutton = gtk_button_new_with_label("Quit");
  g_signal_connect (G_OBJECT (qbutton), "pressed", G_CALLBACK (gtk_main_quit), NULL);

  /* --------------------
     Pack everything
     -------------------- */

  /* --------------------
     Create vbox for packing
     -------------------- */
  
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

  /* --------------------
     Create horizontal button box
     -------------------- */

  GtkWidget *bbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 5);
  gtk_container_add(GTK_CONTAINER(bbox), gobutton);
  gtk_container_add(GTK_CONTAINER(bbox), qbutton);

  //  gtk_box_pack_start(GTK_BOX(vbox), tree, 1, 1, 0);

  GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);

  //   gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), scrolled, 1, 1, 0); // expand and fill both set
  gtk_box_pack_end(GTK_BOX(vbox), bbox, 0, 0, 0);

  gtk_paned_pack1(GTK_PANED(paned), tree, FALSE, TRUE);
  gtk_paned_pack2(GTK_PANED(paned), vbox, FALSE, TRUE);

  gtk_container_add(GTK_CONTAINER(window), paned);

  gtk_widget_show_all (window);
  gtk_main();

  int fd;
  int lock = 0;
  if ((fd = open(dvd->device,O_RDWR|O_NONBLOCK)) == -1)
    {
      printf("Error opening device: \"%s\", ", dvd->device);
      perror("");
      exit(1);
    }
  if (ioctl(fd, CDROM_LOCKDOOR,lock) != 0)
    {
      printf("Error locking device: \"%s\", ",dvd->device);
      perror("");
      exit(1);
    }
  printf("Bye\n");
  exit(0);
}
