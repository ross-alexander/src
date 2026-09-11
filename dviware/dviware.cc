#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>

#include <gdkmm/cairoutils.h>
#include <gtkmm.h>
#include <gtk/gtk.h>

#include <cairomm/cairomm.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>


namespace kpse {
#define NO_DEBUG 1
  extern "C" {
#include <kpathsea/kpathsea.h>
  }
}

#include "common.h"

#define DVI_version 2
#define PK_version 89

#define DVI_set_char_0 0
#define DVI_set_char_127 127
#define DVI_put_rule 137
#define DVI_bop 139
#define DVI_eop 140
#define DVI_push 141
#define DVI_pop 142
#define DVI_right2 144
#define DVI_right3 145
#define DVI_right4 146
#define DVI_w0 147
#define DVI_w1 148
#define DVI_w2 149
#define DVI_w3 150
#define DVI_x0 152
#define DVI_x2 154
#define DVI_x3 155
#define DVI_down3 159
#define DVI_down4 160
#define DVI_y0 161
#define DVI_y1 162
#define DVI_y2 163
#define DVI_y3 164
#define DVI_y4 165
#define DVI_z0 166
#define DVI_z1 167
#define DVI_z2 168
#define DVI_z3 169
#define DVI_fnt_num_0 171
#define DVI_fnt_num_63 234
#define DVI_xxx1 239
#define DVI_fontdef1 243
#define DVI_pre 247
#define DVI_post 248
#define DVI_post_post 249

struct Foo {
  int argc;
  char **argv;
  GtkApplication *app;
};

struct DviWindow {
  char *filename;
  struct DviFile *dvi;
  GtkApplication *app;
  GtkWidget *window;
  int loaded, displayed;
  int page;
  int debug;
};

enum {
  FILE_NAME_COLUMN = 0,
  FILE_LOADED_COLUMN,
  DVI_REF_COLUMN,
  NUM_COLUMNS,
};

/* ----------------------------------------------------------------------
--
-- DviPage
--
---------------------------------------------------------------------- */
DviPage::DviPage(DviFile *f, int o)
{
  file = f;
  offset = o;
}

/* ----------------------------------------------------------------------
--
-- DviLoadFile
--
---------------------------------------------------------------------- */

DviFile* DviLoadFile(char *file)
{
  struct stat statbuf;
  DviFile *dvi;
  FILE *fstream;
  unsigned int commentLen, cmd, pages;
  signed int i;
  uint32_t postamble;
  int32_t offset;

  dvi = new DviFile;

  if (stat(file, &statbuf))
    {
      fprintf(stderr, "Error statting file %s: %s\n", file, strerror(errno));
      delete dvi;
      return NULL;
    }
  dvi->size = statbuf.st_size;

  if ((fstream = fopen(file, "rb")) == NULL)
    {
      fprintf(stderr, "Error opening file %s: %s\n", file, strerror(errno));
      delete dvi;
      return NULL;
    }

  DviStream *stream = new DviStream(fstream);

  uint8_t dvi_pre_val = stream->u8();

  if (dvi_pre_val != DVI_pre)
    {
      fprintf(stderr, "%s: Does not look like a DVI file (no pre command).\n", file);
      delete dvi;
      return NULL;
    }

  uint8_t dvi_ver = stream->u8();
  if (dvi_ver != DVI_version)
    {
      fprintf(stderr, "%s: Version of DVI file %d not equal to %d.\n", file, dvi_ver, DVI_version);
      delete dvi;
      return NULL;
    }

  dvi->num = stream->u32();
  dvi->den = stream->u32();
  dvi->mag = stream->u32();

  /* num and den are top and bottom of fractions which convert all DVI
     dimensions into units of 10^-7 meters ie 0.1 microns.  There are
     7227 TeX points in 254 centimeters and TeX82 works with scaled
     points where 2^16 sp in a point, so TeX82 sets num=25400000 and
     den = 7227*2^16 = 47362672.
  */

  /* mag is what TeX82 calls \mag i.e 1000 * the desired magnification.  Dimensions should be multiplied by mn/1000d. */

  commentLen = stream->u8();
  dvi->comment = stream->String(commentLen);
  printf("%s\n", dvi->comment);
  if (dvi->size < commentLen + 15)
    {
      fprintf(stderr, "%s: File looks truncated.\n", file);
      delete dvi;
      return NULL;
    }

  /* --------------------
     DVI files should be padded with 4 or more bytes of the decimal 223 to pad the file to 8 octets.
     -------------------- */

  for (i = 1; i < 9; i++)
    {
      if (stream->seek(-i, SEEK_END) == -1)
	{
	  fprintf(stderr, "%s: %s\n", file, strerror(errno));
	  delete dvi;
	  return NULL;
	}
      cmd = stream->u8();
      if (cmd != 223)
	break;
    }
  if (i == 9)
    {
      fprintf(stderr, "%s: DVI file has corrupted postamble.\n", file);
      delete dvi;
      return NULL;
    }
  if (cmd != DVI_version)
    {
      fprintf(stderr, "%s: Incorrect version (%d != %d).\n", file, DVI_version, cmd);
      delete dvi;
      return NULL;
    }
  
  stream->seek(-i-4, SEEK_END);
  postamble = stream->u32();

  printf("postamble %d\n", postamble);
  
  stream->seek(postamble, SEEK_SET);
  cmd = stream->u8();

  if (cmd != DVI_post)
    {
      fprintf(stderr, "%s: Could not find postamble.\n", file);
      delete dvi;
      return NULL;
    }
  offset = stream->u32();
  dvi->num = stream->u32(); // numereator
  dvi->den = stream->u32(); // denominator
  dvi->mag = stream->u32();
  dvi->maxv = stream->u32();
  dvi->maxh = stream->u32();
  dvi->maxs = stream->u16();
  dvi->maxp = stream->u16();
  dvi->fonts = new DviFont*[256];

  /* There are 7227 TeX Points in 254 cm (100 inch).  Num and Dem are
     multipliers to convert TeX widths to 10^-7 meters ie 0.1
     microns. mag is magnification * 1000.

     Res is in dots per inch, so 
  */

  while ((cmd = stream->u8()) != DVI_post_post)
    {
      if (cmd == DVI_fontdef1)
	{
	}
    }
  dvi->pages = new DviPage*[dvi->maxp];
  for (pages = 0; offset != -1; pages++)
    {
      int page = dvi->maxp - pages - 1;
      stream->seek(offset, SEEK_SET);
      cmd = stream->u8();
      if (cmd != DVI_bop)
	{
	  fprintf(stderr, "%s: Command at offset %d not bop.\n", file, (int)offset);
	  delete dvi;
	  return NULL;
	}
      dvi->pages[page] = new DviPage(dvi, offset);
      dvi->pages[page]->offset = offset;
      stream->seek(40, SEEK_CUR);
      offset = stream->u32();
    }
  printf("%d pages found.\n", pages);
  dvi->stream = stream;
  return dvi;
}

/* ----------------------------------------------------------------------
--
-- DviDisplayPage
--
---------------------------------------------------------------------- */

int DviDisplayPage(DviFile *dvi, int pnum, int res, Cairo::RefPtr<Cairo::Context> context)
{
  int i;
  int cmd, offset;
  int counters[10];
  int previous_page;
  dvi->conv = (dvi->num/254000.0) * ((double)res/dvi->den) * (dvi->mag / 1000.0);
  dvi->tfm_conv = (double)(25400000.0/dvi->num)*(dvi->den/473628672)/16.0;
  printf("Res = %d num = %d den = %d mag = %d conv = %f\n", res, dvi->num, dvi->den, dvi->mag, dvi->conv);

  DviState stack[10], state;
  int sptr = 0;
  DviFont *font = NULL;

  int a, b, v1, h1, x1, y1, z1;

  int width = (int)(dvi->maxh * dvi->conv);
  int height = (int)(dvi->maxv * dvi->conv);
  
  printf("DviDisplayPage(%d: %d [%d] x %d [%d] [%s])\n", pnum, dvi->maxh, width, dvi->maxv, height, context ? "true":"false");

  if (context)
    {
      context->set_source_rgb(0, 1, 1);
      context->rectangle(0, 0, width, height);
      context->fill();
    }
  
  DviCairo cairo(dvi->maxh, dvi->maxv, dvi->conv, context);
  DviPage *page;
  

  if (pnum < 1 || pnum > dvi->maxp)
    {
      fprintf(stderr, "Illegal page %d.\n", pnum);
      return 0;
    }
  if ((page = dvi->pages[pnum-1]) == NULL)
    {
      fprintf(stderr, "Fatal error: page %d NULL.\n", pnum);
      return 0;
    }
  dvi->stream->seek(page->offset, SEEK_SET);
  offset = dvi->stream->tell();
  cmd = dvi->stream->u8();
  if (cmd != DVI_bop)
    {
      fprintf(stderr, "Fatal error: command at %ld not bop.\n", (long)page->offset);
      return 0;
    }
  for (i = 0; i < 10; i++)
    counters[i] = dvi->stream->u32();

  previous_page = dvi->stream->u32();

  printf("%d: begining of page %d [prev %d]\n", offset, counters[0], previous_page);

  while ((offset = dvi->stream->tell()) && ((cmd = dvi->stream->u8()) != DVI_eop))
    {
      if (cmd >= DVI_set_char_0 && cmd <= DVI_set_char_127)
	{
	  if (font->glyph[cmd].flags == 0)
	    {
	      printf("Missing glyph %d\n", cmd);
	    }
	  else
	    {
	      printf("%d: set_char_%d ", offset, cmd);
	      DviGlyph *g = font->glyph + cmd;
	      
	      //	      Cairo::RefPtr<Cairo::SurfacePattern::SurfacePattern> pattern = Cairo::SurfacePattern::create(g->surface);

	      //	      cr->set_source(font->glyph[cmd].surface, 0, 0);
	      int tfm_width = ((long long)g->tfm * font->scaled_size) >> 20;
	      printf("h:=%d+%d=%d\n", state.h, tfm_width, state.h + tfm_width);
	      cairo.Place(state.h, state.v, g);
	      state.h += tfm_width;
	      state.v += 0;
	    }
	}
      else if (cmd >= DVI_fnt_num_0 && cmd <= DVI_fnt_num_63)
	{
	  font = dvi->fonts[cmd - DVI_fnt_num_0];
	  printf("%d: fnt_num_%d (%s)\n", offset, cmd - DVI_fnt_num_0, font->name);
	}
      else switch (cmd)
	{
	case DVI_put_rule:
	  a = dvi->stream->u32();
	  b = dvi->stream->u32();
	  printf("%d: put_rule %d %d -- NOT IMPLEMENTED\n", offset, a, b);
	  break;
	case DVI_down3:
	  v1 = dvi->stream->s24();
	  state.v += v1;
	  printf("%d: down3 %d\n", offset, v1);
	  break;
	case DVI_down4:
	  v1 = dvi->stream->s32();
	  printf("%d: down4 %d v:=%d+%d=%d vv:=%d\n", offset, v1, state.v, v1, state.v + v1, (int)(v1 * dvi->conv));
	  state.v += v1;
	  break;
	case DVI_push:
	  printf("%d: push\n", offset);
	  stack[sptr] = state;
	  sptr++;
	  break;
	case DVI_pop:
	  printf("%d: pop\n", offset);
	  sptr--;
	  state = stack[sptr];
	  break;
	case DVI_right2:
	  h1 = dvi->stream->s16();
	  state.h += h1;
	  printf("%d: right2 %d\n", offset, h1);
	  break;
	case DVI_right3:
	  h1 = dvi->stream->s24();
	  state.h += h1;
	  printf("%d: right3 %d\n", offset, h1);
	  break;
	case DVI_right4:
	  h1 = dvi->stream->u32();
	  state.h += h1;
	  printf("%d: right4 %d\n", offset, h1);
	  break;
	case DVI_w0:
	  printf("%d: w0\n", offset);
	  state.h += state.w;
	  break;
	case DVI_w1:
	  h1 = dvi->stream->u8();
	  state.w = h1;
	  state.h += state.w;
	  printf("%d: w1 %d\n", offset, h1);
	  break;
	case DVI_w2:
	  h1 = dvi->stream->s16();
	  state.w = h1;
	  state.h += state.w;
	  printf("%d: w2 %d\n", offset, h1);
	  break;
	case DVI_w3:
	  h1 = dvi->stream->s24();
	  state.w = h1;
	  state.h += state.w;
	  printf("%d: w3 %d\n", offset, h1);
	  break;
	case DVI_x0:
	  printf("%d: x0\n", offset);
	  state.h += state.x;
	  break;
	case DVI_x2:
	  printf("%d: x2\n", offset);
	  x1 = dvi->stream->s16();
	  state.x = x1;
	  state.h += state.x;
	  break;
	case DVI_x3:
	  printf("%d: x3\n", offset);
	  x1 = dvi->stream->s24();
	  state.x = x1;
	  state.h += state.x;
	  break;

	case DVI_y0:
	  printf("%d: y0\n", offset);
	  state.v += state.y;
	  break;
	case DVI_y3:
	  printf("%d: y3\n", offset);
	  y1 = dvi->stream->s24();
	  state.y = y1;
	  state.v += state.y;
	  break;

	case DVI_z0:
	  printf("%d: z0\n", offset);
	  state.v += state.z;
	  break;
	case DVI_z3:
	  printf("%d: z3\n", offset);
	  z1 = dvi->stream->s24();
	  state.z = z1;
	  state.v += state.z;
	  break;

	case DVI_fontdef1:
	  {
	    DviFont *font = new DviFont(dvi->stream);
	    dvi->fonts[font->id] = font;
	    font->Load(res);
	    break;
	  }
	case DVI_xxx1:
	  {
	    int k = dvi->stream->u8();
	    for (int i = 0; i < k; i++)
	      {
		int j = dvi->stream->u8();
		fputc(j, stdout);
	      }
	    break;
	  }
	default:
	  printf("%d: %d\n", offset, cmd);
	  exit(1);
	}
    }
  printf("\n");
  if (!context)
    {
      cairo.Save("dvi.png");
    }
  return 1;
}


/* ----------------------------------------------------------------------
--
--
--
---------------------------------------------------------------------- */

static void draw_event (GtkDrawingArea *drawing_area, cairo_t* cr, int width, int height, gpointer data)
{

  Cairo::RefPtr<Cairo::Context> context = Gdk::Cairo::wrap(cr, false);

  DviWindow *dviw = (DviWindow*)data;
  
  context->set_source_rgb(0, 0, 0);
  context->rectangle(0, 0, width, height);
  context->fill();

  DviDisplayPage(dviw->dvi, 1, 600, context);
}
		   
gboolean expose_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  printf("Expose called.\n");
  return 1;
}

gboolean configure_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  printf("Configure called.\n");
  return 1;
}

gboolean realize_event(GtkWidget *widget,  gpointer data)
{
  struct DviWindow *dviw = (DviWindow*)data;

  printf("Realize called %s.\n", dviw->filename);
  // DviDisplayPage(dviw->dvi, dviw->page, 600);
  return 1;
}

/* ----------------------------------------------------------------------
--
--
--
---------------------------------------------------------------------- */

static int selection_changed (GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, gpointer data)
{
  GtkTreeIter iter;
  gchar *filename;
  struct DviWindow *dviw;

  G_GNUC_BEGIN_IGNORE_DEPRECATIONS;

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, FILE_NAME_COLUMN, &filename, -1);
  gtk_tree_model_get (model, &iter, DVI_REF_COLUMN, &dviw, -1);

  G_GNUC_END_IGNORE_DEPRECATIONS;

  printf("Selection %s %d %s %p.\n", filename, path_currently_selected, (char*)data, dviw);

  if (!dviw->loaded)
    {
      if ((dviw->dvi = DviLoadFile(dviw->filename)) == NULL)
	{
	  fprintf(stderr, "Failed to load %s\n", dviw->filename);
	  return 0;
	}
      dviw->loaded = 1;
      dviw->page = 1;
    }
  if (!dviw->displayed)
    {
      GtkWidget *window, *drawing;

      window = gtk_application_window_new (dviw->app);
      //      gtk_container_set_border_width (GTK_CONTAINER (window), 5);

      drawing = gtk_drawing_area_new();
      gtk_widget_set_size_request(drawing, 400, 300);
      gtk_window_set_child(GTK_WINDOW(window), drawing);

      gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing), draw_event, dviw, NULL);


      g_signal_connect(G_OBJECT(drawing), "realize", G_CALLBACK(realize_event), dviw);
      // g_signal_connect(G_OBJECT(drawing), "configure_event", G_CALLBACK(configure_event), dviw);
      // g_signal_connect(G_OBJECT(drawing), "draw", G_CALLBACK(draw_event), dviw;
      
      dviw->window = window;
      dviw->displayed = 1;
      printf("%p %p\n", dviw, dviw->dvi);
      gtk_widget_set_visible(window, 1);
    }
  return 1;
}

/* ----------------------------------------------------------------------
--
--
--
---------------------------------------------------------------------- */

static void add_columns (GtkTreeView *treeview)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  G_GNUC_BEGIN_IGNORE_DEPRECATIONS

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Filename",
						     renderer,
						     "text", FILE_NAME_COLUMN,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, FILE_NAME_COLUMN);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);

  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes ("Loaded",
						     renderer,
						     "active", FILE_LOADED_COLUMN,
						     NULL);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				   GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column (treeview, column);

  G_GNUC_END_IGNORE_DEPRECATIONS
}

static void activate (GtkApplication *app, gpointer user_data)
{
  Foo *foomatic = (Foo*)user_data;
  
  GtkListStore *store;
  GtkTreeIter iter;
  GtkWidget *window, *treeview;
  GtkTreeSelection *selection;

  G_GNUC_BEGIN_IGNORE_DEPRECATIONS;

  store = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_POINTER);
  
  G_GNUC_END_IGNORE_DEPRECATIONS;
    
  for (int i = 1; i < foomatic->argc; i++)
    {
      DviWindow *dviw = new DviWindow;
      dviw->filename = foomatic->argv[i];
      dviw->loaded = 0;
      dviw->displayed = 0;
      dviw->app = app;
      G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter,
			 FILE_NAME_COLUMN, dviw->filename,
			 FILE_LOADED_COLUMN, FALSE,
			 DVI_REF_COLUMN, dviw,
			 -1);
      G_GNUC_END_IGNORE_DEPRECATIONS;
    }

  //  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  window = gtk_application_window_new (app);

  // g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (gtk_main_quit), NULL);

  //  gtk_container_set_border_width (GTK_CONTAINER (window), 5);

  G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
  treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(store));

    // gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
  
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
				   FILE_NAME_COLUMN);
  g_object_unref(G_OBJECT(store));
  add_columns(GTK_TREE_VIEW(treeview));

  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
  gtk_tree_selection_unselect_all(selection);
  gtk_tree_selection_set_select_function(selection, selection_changed, (void*)"Testing", NULL);
  gtk_window_set_child(GTK_WINDOW(window), treeview);

  G_GNUC_END_IGNORE_DEPRECATIONS;

  gtk_window_set_default_size (GTK_WINDOW (window), 280, 250);
  gtk_widget_set_visible(window, 1);
}

/* ----------------------------------------------------------------------
--
--
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  kpse::kpse_set_program_name(argv[0], NULL);
  kpse::kpse_init_prog(argv[0], 600, "ljfour", NULL);

  for (int i = 1; i < argc; i++)
    {
      struct DviFile *dvi = DviLoadFile(argv[i]);
      assert(dvi);
      DviDisplayPage(dvi, 1, 600, nullptr);
    }

  Foo foomatic;

  GtkApplication *app;
  app = gtk_application_new("net.hepazulian.xtk", G_APPLICATION_DEFAULT_FLAGS);

  foomatic.argc = argc;
  foomatic.argv = argv;
  foomatic.app = app;
  
  g_signal_connect(app, "activate", G_CALLBACK (activate), &foomatic);

  int status = g_application_run(G_APPLICATION(app), 0, argv);
  g_object_unref(app);
  return status;
}

