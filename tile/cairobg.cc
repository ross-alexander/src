/* ----------------------------------------------------------------------
--
-- cairobg
--
-- 2019-03-04: Ross Alexander
--
---------------------------------------------------------------------- */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_XCB_XCB_H
#include <xcb/xcb.h>
#endif

#ifdef HAVE_XCB_XCBAUX_H
#include <xcb/xcb_aux.h>
#endif

#ifdef HAVE_CAIRO_H
#include <cairo.h>
#endif

#ifdef HAVE_CAIROXLIB_H
#include <cairo-xlib.h>
#endif

#ifdef HAVE_CAIROXCB_H
#include <cairo-xcb.h>
#endif

#ifdef HAVE_GDK30_H
#include <gdk/gdk.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <vector>
#include <string>
#include <algorithm>
#include <lua.hpp>

/* ----------------------------------------------------------------------
--
-- scanner_t
--
---------------------------------------------------------------------- */

class scanner_t {
public:
  std::vector<std::string> dir;
  std::vector<std::string> file;
  int index;
  scanner_t& operator+=(const char*);
  scanner_t& operator+=(std::string);
  void rescan_dir(std::string);
  int rescan();
  const char* random();
};

int scanner_t::rescan()
{
    /* --------------------
     Clear any existing list
     -------------------- */
  
  file.clear();
  
  for (unsigned i = 0; i < dir.size(); i++)
    {
      rescan_dir(dir[i]);
    }
  time_t t = time(0);
  srand(t);
  std::random_shuffle(file.begin(), file.end());
  index = 0;
  return 1;
}

const char* scanner_t::random()
{
  if (file.size() == 0)
    return nullptr;

  const char *s = file[index].c_str();
  index = (index+1) % file.size();
  return s;
}

void scanner_t::rescan_dir(std::string dirpath)
{
  DIR *dirp;
  unsigned flen = pathconf(dirpath.c_str(), _PC_NAME_MAX) + 1;
  unsigned dlen = offsetof(struct dirent, d_name) + flen;
  char *path = (char*)calloc(flen, 1);
  struct dirent **namelist;
  struct stat statbuf;
  int dcount;
  
  if ((dcount = scandir(dirpath.c_str(), &namelist, 0, alphasort)) >= 0)
    {
      for (int i = 0; i < dcount; i++)
	{
	  snprintf(path, flen, "%s/%s", dirpath.c_str(), namelist[i]->d_name);
	  if (stat(path, &statbuf) == 0)
	    {
	      if (S_ISREG(statbuf.st_mode))
		{
		  file.push_back(std::string(path));
		}
	      if (S_ISDIR(statbuf.st_mode) && (strcmp(".", namelist[i]->d_name)) && (strcmp("..", namelist[i]->d_name)))
		{
		  rescan_dir(path);
		}
	    }
	}
      free(namelist);
    }
  else
    {
      fprintf(stderr, "scandir %s failed: %s\n", dirpath, strerror(errno));
    }
  free(path);
} 


scanner_t& scanner_t::operator+=(const char *d)
{
  dir.push_back(std::string(d));
  return *this;
}

scanner_t& scanner_t::operator+=(std::string d)
{
  dir.push_back(d);
  return *this;
}

/* ----------------------------------------------------------------------
--
-- bg_surface
--
---------------------------------------------------------------------- */

class bg_surface_t {
public:
  cairo_surface_t *surface;
};

class bg_surface_png_t : bg_surface_t {
  const char *path;
};


/* ----------------------------------------------------------------------
--
-- bg_gdk
--
-- Use GDK-Pixbuf to load image into drawable
--
---------------------------------------------------------------------- */

int bg_gdk(cairo_surface_t *surface, int width_surface, int height_surface, const char *file)
{
  GError *error = 0;
  GdkPixbuf *px;
  GdkPixbufFormat *format;
  int width_image;
  int height_image;
  
  if ((file == nullptr) || ((format = gdk_pixbuf_get_file_info(file, &width_image, &height_image)) == nullptr))
    {
      return 0;
    }

  double scale;

  if ((double)width_surface/(double)width_image > ((double)height_surface/height_image))
    scale = (double)height_surface/(double)height_image;
  else
    scale = (double)width_surface/(double)width_image;

  int width_scaled = width_image * scale;
  int height_scaled = height_image * scale;

  /* If image is scalable then load it at the correct scale */
  
  if (gdk_pixbuf_format_is_scalable(format))
    px = gdk_pixbuf_new_from_file_at_scale(file, width_scaled, height_scaled, 1, &error);
  else
    px = gdk_pixbuf_new_from_file(file, &error);
  
  int depth = gdk_pixbuf_get_bits_per_sample(px);
  int alpha = gdk_pixbuf_get_has_alpha(px);

  GdkColorspace color = gdk_pixbuf_get_colorspace(px);
  printf("Image %s size %d x %d x %d [alpha %d scalable %d] %s\n", file, width_image, height_image, depth, alpha, gdk_pixbuf_format_is_scalable(format), gdk_pixbuf_format_get_name(format));
  printf("Screen %dx%d Image %dx%d Scale %dx%d\n", width_surface, height_surface, width_image, height_image, width_scaled, height_scaled);


  if (!gdk_pixbuf_format_is_scalable(format))
    {
      GdkPixbuf *spx = gdk_pixbuf_new(color, alpha, depth, width_scaled, height_scaled);
      gdk_pixbuf_fill(spx, 0xffffffff);
      gdk_pixbuf_scale(px, spx, 0, 0, width_scaled, height_scaled, 0.0, 0.0, scale, scale, GDK_INTERP_HYPER);
      g_object_unref(px);
      px = spx;
    }

  cairo_t* cr = cairo_create(surface);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_rectangle(cr, 0, 0, width_surface, height_surface);
  cairo_fill(cr);
  gdk_cairo_set_source_pixbuf(cr, px, (width_surface-width_scaled)/2, (height_surface-height_scaled)/2);
  cairo_paint(cr);
  cairo_show_page(cr);
  cairo_destroy(cr);
  return 1;
}
  
void bg_cairo(cairo_surface_t *surface, int width, int height)
{
  cairo_t* cr = cairo_create(surface);
  assert(cr);
  
  cairo_pattern_t *p = cairo_pattern_create_linear(0, 0, width, height);
  cairo_pattern_add_color_stop_rgba(p, 0, 0, 1, 0, 1);
  cairo_pattern_add_color_stop_rgba(p, 1, 0, 0, 1, 1);
  cairo_set_source(cr, p);
  cairo_new_path(cr);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_fill(cr);
  cairo_show_page(cr);
  cairo_destroy(cr);
}

/* ----------------------------------------------------------------------
--
-- surface_render
--
---------------------------------------------------------------------- */

void surface_render(std::vector<std::string>& args, cairo_surface_t* surface, int width, int height)
{
  if (args.size() > 0)
    {
      scanner_t s;
      for (auto arg : args)
	{
	  s += arg;
	}
      s.rescan();
      const char* path = s.random();
      if (!bg_gdk(surface, width, height, path))
	{
	  fprintf(stderr, "GDK load failed, using default\n");
	  bg_cairo(surface, width, height);
	}
    }
  else
    {
      bg_cairo(surface, width, height);
    }
}

/* ----------------------------------------------------------------------
--
-- render_xlib_root
--
---------------------------------------------------------------------- */

void render_xlib_root(std::vector<std::string> &args)
{
  unsigned int width = 0;
  unsigned int height = 0;
  unsigned int depth = 0;

  Display *dpy;
  dpy = XOpenDisplay(NULL);
  assert(dpy != NULL);
  
  Screen *screen = XDefaultScreenOfDisplay(dpy);
  Drawable root = screen->root;
  
  int rootx, rooty;
  unsigned int rootbw;
  
  XGetGeometry(dpy, root, &root, &rootx, &rooty, &width, &height, &rootbw, &depth);	
  Visual *v = XDefaultVisualOfScreen(screen);
  
  cairo_surface_t *surface = cairo_xlib_surface_create(dpy, root, v, width, height);
  surface_render(args, surface, width, height);
  cairo_surface_destroy(surface);
  XCloseDisplay(dpy);
}
		      

/* ----------------------------------------------------------------------
--
-- render_xlib_root
--
---------------------------------------------------------------------- */

void render_xcb(std::vector<std::string> &args, int use_root)
{
  cairo_surface_t *surface = 0;
  unsigned int width = 0;
  unsigned int height = 0;
  unsigned int depth = 0;
  
  int screenNum;
  xcb_connection_t *connection;
  connection = xcb_connect(NULL, &screenNum);
  assert(connection);
  
 /* Get the screen whose number is screenNum */
 
  const xcb_setup_t *setup = xcb_get_setup(connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);  
      
      // we want the screen at index screenNum of the iterator
  for (int i = 0; i < screenNum; ++i)
    xcb_screen_next(&iter);
  
  xcb_screen_t *screen = iter.data;
  assert(screen);
      
  xcb_visualtype_t* visual = xcb_aux_find_visual_by_attrs(screen, XCB_VISUAL_CLASS_TRUE_COLOR, 24);
  //      xcb_visualtype_t* visual = xcb_aux_find_visual_by_id(screen, screen->root_visual);
  assert(visual);
      
  width = screen->width_in_pixels;
  height = screen->height_in_pixels;
  depth = screen->root_depth;
  xcb_window_t root_window = screen->root;


  if (use_root)
    {
      xcb_pixmap_t background_pixmap = xcb_generate_id(connection);
      xcb_create_pixmap(connection, depth, background_pixmap, root_window, width, height);
      surface = cairo_xcb_surface_create(connection, background_pixmap, visual, width, height);
      surface_render(args, surface, width, height);

      printf("render_xcb_root\n");
#ifdef UseGC
      uint32_t values[2];
      int gc_mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
      values[0] = screen->black_pixel;
      values[1] = screen->white_pixel;
      
      xcb_gc_t gc = xcb_generate_id(connection);
      xcb_create_gc(connection, gc, xcb_pm, gc_mask, values);
      xcb_copy_area(connection, xcb_pm, screen->root, gc, 0, 0, 0, 0, width, height);
#else
      xcb_params_cw_t cw;
      cw.back_pixmap = background_pixmap;
      xcb_aux_change_window_attributes(connection, screen->root, XCB_CW_BACK_PIXMAP, &cw);
      xcb_void_cookie_t cookie = xcb_clear_area_checked(connection, 0, screen->root, 0, 0, width, height);
      xcb_request_check(connection, cookie);
#endif
      xcb_flush(connection);
    }
  else
    {
      printf("render_xcb_window\n");

      int window_width = width/2;
      int window_height = height/2;
      
      uint32_t new_cw_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
      uint32_t new_cw_values[2];
      new_cw_values[0] = screen->white_pixel;
      new_cw_values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_BUTTON_PRESS;
      xcb_window_t window = xcb_generate_id (connection);
      xcb_create_window (connection, XCB_COPY_FROM_PARENT, window, screen->root,
			 10, 10, window_width, window_height, 1,
			 XCB_WINDOW_CLASS_INPUT_OUTPUT,
			 visual->visual_id,
			 new_cw_mask, new_cw_values);

      xcb_pixmap_t background_pixmap = xcb_generate_id(connection);
      xcb_create_pixmap(connection, depth, background_pixmap, window, window_width, window_height);
      surface = cairo_xcb_surface_create(connection, background_pixmap, visual, window_width, window_height);
      surface_render(args, surface, window_width, window_height);

      xcb_map_window(connection, window);
      xcb_params_cw_t cw;
      cw.back_pixmap = background_pixmap;
      xcb_aux_change_window_attributes(connection, window, XCB_CW_BACK_PIXMAP, &cw);
      xcb_void_cookie_t cookie = xcb_clear_area_checked(connection, 0, window, 0, 0, window_width, window_height);
      xcb_request_check(connection, cookie);
      xcb_flush(connection);
      sleep(5);
    }
  cairo_surface_destroy(surface);
  xcb_disconnect(connection);
}


/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char* const argv[])
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  
  int use_xlib = 0;
  int use_lua = 0;
  int use_root = 1;
  int ch;
  while ((ch = getopt(argc, argv, "l:w")) != EOF)
    switch(ch)
      {
      case 'l':
	{
	  printf("%s: calling lua file %s\n", argv[0], optarg);
	  if (!luaL_dofile(L, optarg))
	    {
	      printf("%s: %s failed to load\n", argv[0], optarg);
	      exit(1);
	    }
	  use_lua = 1;
	  break;
	}
      case 'w':
	use_root = 0;
	break;
      }

  /* --------------------
     -- Add args to lua
     -------------------- */

  lua_pushglobaltable(L);
  lua_newtable(L);
  
  for (int i = optind; i < argc; i++)
    {
      lua_pushstring(L, argv[i]);
      lua_rawseti(L, -2, i - optind + 1);
    }
  lua_setfield(L, -2, "args");
  
  lua_pushglobaltable(L);
  lua_getfield(L, -2, "init");
  if (lua_isfunction(L, -1))
    {
      printf("%s: calling luafunction init\n", argv[0]);
      lua_call(L, 0, 0);
    }
  else
    {
      lua_pop(L, 1);
    }

  std::vector<std::string> args;
  for (int i = optind; i < argc; i++)
    args.push_back(std::string(argv[i]));

  /* --------------------
     Use xlib
     -------------------- */

  if (use_xlib)
    render_xlib_root(args);
  else
    render_xcb(args, use_root);
}
