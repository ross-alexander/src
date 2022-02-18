#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
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

#ifdef HAVE_GDK30_H
#include <gdk/gdk.h>
#endif

#ifdef HAVE_CAIROXCB_H
#include <cairo-xcb.h>
#endif

#include <lua.hpp>
#include <string>
#include "tile.h"

/* ----------------------------------------------------------------------
--
-- tile_surface_xcb_t
--
---------------------------------------------------------------------- */

tile_surface_xcb_t::tile_surface_xcb_t()
{
  int screenNum;
  conn = xcb_connect(nullptr, &screenNum);
  assert(conn);
  
  /* Get the screen whose number is screenNum */
  
  const xcb_setup_t *setup = xcb_get_setup(conn);
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
  root_drawable = screen->root;

  fprintf(stdout, "screen width x height x depth (%d x %d x %d)\n", width, height, depth);
  
  xcb_pixmap_t background_pixmap = xcb_generate_id(conn);
  xcb_create_pixmap(conn, depth, background_pixmap, root_drawable, width, height);

  xcb_params_cw_t cw;
  cw.back_pixmap = background_pixmap;
  xcb_aux_change_window_attributes(conn, root_drawable, XCB_CW_BACK_PIXMAP, &cw);
  surface = cairo_xcb_surface_create(conn, background_pixmap, visual, width, height);
}

tile_surface_xcb_t::~tile_surface_xcb_t()
{
  if (conn)
    {
      xcb_void_cookie_t cookie = xcb_clear_area_checked(conn, 0, root_drawable, 0, 0, width, height);
      xcb_request_check(conn, cookie);
      xcb_disconnect(conn);
    }
}

/* ----------------------------------------------------------------------
--
-- new_from_xcb
--
---------------------------------------------------------------------- */

int tile_surface_t_new_from_xcb(lua_State *L)
{
  tile_surface_t** surfacep = (tile_surface_t**)lua_newuserdata(L, sizeof(tile_surface_t*));
 
  *surfacep = new tile_surface_xcb_t();
  luaL_getmetatable(L, "tile_surface_t");
  lua_setmetatable(L, -2);
  return 1;
}
