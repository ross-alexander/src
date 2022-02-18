/* ----------------------------------------------------------------------
--
-- tile
--
-- 2019-03-21: Ross Alexander
--   Move depth to tile_surface_t and set it in the constructors
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
#include <fmt/format.h>
#include <fmt/printf.h>

#include "tile.h"

/* ----------------------------------------------------------------------
--
-- tile_surface_t
--
---------------------------------------------------------------------- */

tile_surface_t::~tile_surface_t()
{
  cairo_surface_destroy(surface);
}

tile_surface_t::tile_surface_t()
{
  surface = nullptr;
}


tile_surface_t::tile_surface_t(uint32_t w, uint32_t h)
{
  width = w;
  height = h;
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  width = cairo_image_surface_get_width(surface);
}

tile_surface_t::tile_surface_t(tile_pixbuf_t *pixbuf)
{
  assert(pixbuf->pixbuf);
  width = gdk_pixbuf_get_width(pixbuf->pixbuf);
  height = gdk_pixbuf_get_height(pixbuf->pixbuf);
  depth = gdk_pixbuf_get_bits_per_sample(pixbuf->pixbuf) * gdk_pixbuf_get_n_channels(pixbuf->pixbuf);
  
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  
  cairo_t *cairo = cairo_create(surface);
  gdk_cairo_set_source_pixbuf(cairo, pixbuf->pixbuf, 0, 0);
  cairo_rectangle(cairo, 0, 0, width, height);
  cairo_fill(cairo);
  cairo_destroy(cairo);
}

int tile_surface_t___gc(lua_State *L)
{
  luaL_checkudata(L, 1, "tile_surface_t");
  tile_surface_t* surface = *(tile_surface_t**)lua_touserdata(L, 1);
  delete surface;
  return 0;
}

void tile_surface_t::fill(double r, double g, double b)
{
  cairo_t *cairo = cairo_create(surface);
  cairo_set_source_rgb(cairo, r, g, b);
  //  cairo_rectangle(cairo, 0, 0, width, height);
  //  cairo_fill(cairo);
  cairo_paint(cairo);
  cairo_destroy(cairo);
}

void tile_surface_t::compose(int32_t w, int32_t h, tile_pixbuf_t *px)
{
  if (px->pixbuf == nullptr)
    return;
  cairo_t *cairo = cairo_create(surface);
  gdk_cairo_set_source_pixbuf(cairo, px->pixbuf, w, h);
  cairo_paint(cairo);
  //  cairo_rectangle(cairo, w, h, gdk_pixbuf_get_width(px->pixbuf), gdk_pixbuf_get_height(px->pixbuf));
  //  cairo_fill(cairo);
  cairo_destroy(cairo);
}

int tile_surface_t___tostring(lua_State *L)
{
  tile_surface_t *surface = *(tile_surface_t**)lua_touserdata(L, 1);

  std::string s = fmt::sprintf("tile_surface_t(%d x %d x %d)", surface->width, surface->height, surface->depth);
  lua_pushstring(L, s.c_str());
  return 1;
}

int tile_surface_t_fill(lua_State *L)
{
  assert(lua_gettop(L) == 4);
  tile_surface_t *surface = *(tile_surface_t**)lua_touserdata(L, 1);
  surface->fill(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
  return 0;
}  

int tile_surface_t_to_file(lua_State *L)
{
  assert(lua_gettop(L) == 2);
  assert(lua_isuserdata(L, 1));
  assert(lua_isstring(L, 2));
  tile_surface_t *surface = *(tile_surface_t**)lua_touserdata(L, 1);

  const char* path = lua_tostring(L, 2);
  cairo_surface_write_to_png(surface->surface, path);
  return 0;
}

int tile_surface_t_new(lua_State *L)
{
  assert(lua_gettop(L) == 2);
  assert(lua_isinteger(L, 1));
  assert(lua_isinteger(L, 2));

  tile_surface_t** surfacep = (tile_surface_t**)lua_newuserdata(L, sizeof(tile_surface_t*));
  *surfacep = new tile_surface_t(lua_tointeger(L, 1), lua_tointeger(L, 2));
  lua_getfield(L, LUA_REGISTRYINDEX, "tile_surface_t");
  lua_setmetatable(L, -2);
  return 1;
}

int tile_surface_t_new_from_pixbuf(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  assert(luaL_checkudata(L, 1, "tile_pixbuf_t"));

  tile_pixbuf_t *pixbuf = *(tile_pixbuf_t**)lua_touserdata(L, 1);
  tile_surface_t** surfacep = (tile_surface_t**)lua_newuserdata(L, sizeof(tile_surface_t*));
 
  *surfacep = new tile_surface_t(pixbuf);
  lua_getfield(L, LUA_REGISTRYINDEX, "tile_surface_t");
  lua_setmetatable(L, -2);
  return 1;
}

int tile_surface_t_compose_pixbuf(lua_State *L)
{
  assert(lua_gettop(L) == 4);
  assert(luaL_checkudata(L, 1, "tile_surface_t"));
  assert(lua_isnumber(L, 2));
  assert(lua_isnumber(L, 3));
  assert(luaL_checkudata(L, 4, "tile_pixbuf_t"));

  tile_surface_t *surface = *(tile_surface_t**)lua_touserdata(L, 1);
  tile_pixbuf_t *pixbuf = *(tile_pixbuf_t**)lua_touserdata(L, 4);
  surface->compose(lua_tonumber(L, 2), lua_tonumber(L, 3), pixbuf);
  return 0;
}

int tile_surface_t_width_get(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  luaL_checkudata(L, 1, "tile_surface_t");

  tile_surface_t* surface = *(tile_surface_t**)lua_touserdata(L, 1);
  lua_pushinteger(L, surface->width);
  return 1;
}

int tile_surface_t_height_get(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  luaL_checkudata(L, 1, "tile_surface_t");

  tile_surface_t* surface = *(tile_surface_t**)lua_touserdata(L, 1);
  lua_pushinteger(L, surface->height);
  return 1;
}

int tile_surface_t___index(lua_State *L)
{
  assert(lua_gettop(L) == 2);
  assert(luaL_checkudata(L, 1, "tile_surface_t"));
  const char* key = luaL_checkstring(L, 2);

  lua_pushvalue(L, lua_upvalueindex(1));
  lua_getfield(L, -1, key);
  if(!lua_isnil(L, -1))
    return 1;

  lua_pushvalue(L, lua_upvalueindex(2));
  lua_getfield(L, -1, key);
  
  if (!lua_isnil(L, -1))
    {
      lua_pushvalue(L, 1);
      lua_call(L, 1, 1);
    }
  return 1;
}

int luaopen_tile_surface_t(lua_State *L)
{
    /* --------------------
     Create surface metatable
     -------------------- */

  //  lua_newtable(L);
  luaL_newmetatable(L, "tile_surface_t");
  lua_pushcclosure(L, tile_surface_t___tostring, 0);  lua_setfield(L, -2, "__tostring");
  lua_pushcclosure(L, tile_surface_t___gc, 0);  lua_setfield(L, -2, "__gc");

  /* --------------------
     methods table
     -------------------- */

  lua_newtable(L);
  lua_pushcclosure(L, tile_surface_t_to_file, 0);  lua_setfield(L, -2, "to_file");
  lua_pushcclosure(L, tile_surface_t_fill, 0);  lua_setfield(L, -2, "fill");
  lua_pushcclosure(L, tile_surface_t_compose_pixbuf, 0);  lua_setfield(L, -2, "compose_pixbuf");

  /* --------------------
     getters/setters table
     -------------------- */
  
  lua_newtable(L);

  lua_pushcclosure(L, tile_surface_t_width_get, 0); lua_setfield(L, -2, "width");
  lua_pushcclosure(L, tile_surface_t_height_get, 0); lua_setfield(L, -2, "height");
  lua_pushcclosure(L, tile_surface_t___index, 2); lua_setfield(L, -2, "__index");

  //  lua_setfield(L, LUA_REGISTRYINDEX, "tile_surface_t");
  lua_pop(L, 1);

  /* --------------------
     Create global tile_pixbuf_t table
     -------------------- */

  lua_pushglobaltable(L);

  lua_newtable(L);
  lua_pushcclosure(L, tile_surface_t_new, 0); lua_setfield(L, -2, "new");
  lua_pushcclosure(L, tile_surface_t_new_from_pixbuf, 0); lua_setfield(L, -2, "new_from_pixbuf");
  lua_pushcclosure(L, tile_surface_t_new_from_xcb, 0); lua_setfield(L, -2, "new_from_xcb");

  lua_setfield(L, -2, "tile_surface_t");
  return 1;
}

