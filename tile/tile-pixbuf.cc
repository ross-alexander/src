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
-- tile_pixbuf_t
--
---------------------------------------------------------------------- */

tile_pixbuf_t::tile_pixbuf_t()
{
  path = nullptr;
  pixbuf = nullptr;
}

tile_pixbuf_t::tile_pixbuf_t(GdkPixbuf* px)
{
  path = nullptr;
  pixbuf = px;
}

int tile_pixbuf_t::new_from_file(const char *p)
{
  if (path) free((void*)path);
  if (pixbuf) g_object_unref(pixbuf);

  GError *error = 0;
  pixbuf = gdk_pixbuf_new_from_file(p, &error);
  assert(pixbuf != 0);
  path = strdup(p);
  return 1;
}

int tile_pixbuf_t::new_from_file_at_scale(const char *p, int width, int height, gboolean aspect)
{
  if (path) free((void*)path);
  if (pixbuf) g_object_unref(pixbuf);

  GError *error = 0;
  pixbuf = gdk_pixbuf_new_from_file_at_scale(p, width, height, aspect, &error);
  assert(pixbuf != 0);
  path = strdup(p);
  return 1;
}


tile_pixbuf_t::tile_pixbuf_t(const char *p) : tile_pixbuf_t()
{
  new_from_file(p);
}

tile_pixbuf_t::tile_pixbuf_t(const char *p, int width, int height, gboolean aspect) : tile_pixbuf_t()
{
  new_from_file_at_scale(p, width, height, aspect);
}


tile_pixbuf_t::~tile_pixbuf_t()
{
  if (path) free((void*)path);
  if (pixbuf) g_object_unref(pixbuf);
}

tile_pixbuf_t* tile_pixbuf_t::scale(uint32_t w, uint32_t h)
{
  if (!pixbuf)
    return nullptr;
  GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, w, h, GDK_INTERP_HYPER);
  return new tile_pixbuf_t(scaled);
}

int tile_pixbuf_t___tostring(lua_State *L)
{
  tile_pixbuf_t *pixbuf = *(tile_pixbuf_t**)lua_touserdata(L, 1);
  if (pixbuf->pixbuf)
    {
      int width = gdk_pixbuf_get_width(pixbuf->pixbuf);
      int height = gdk_pixbuf_get_height(pixbuf->pixbuf);
      int alpha = gdk_pixbuf_get_has_alpha(pixbuf->pixbuf);
      int depth = gdk_pixbuf_get_bits_per_sample(pixbuf->pixbuf);
      
      std::string s = fmt::sprintf("tile_pixbuf_t(%s %d x %d x %d)", pixbuf->path ? pixbuf->path : "null", width, height, depth);
      lua_pushstring(L, s.c_str());
    }
  else
    {
      std::string s = fmt::sprintf("tile_pixbuf_t(%s)", pixbuf->path ? pixbuf->path : "null");
      lua_pushstring(L, s.c_str());
    }
  return 1;
}

int tile_pixbuf_t___index(lua_State *L)
{
  assert(lua_gettop(L) == 2);
  assert(luaL_checkudata(L, 1, "tile_pixbuf_t"));
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

int tile_pixbuf_t___gc(lua_State *L)
{
  luaL_checkudata(L, 1, "tile_pixbuf_t");
  tile_pixbuf_t* pixbuf = *(tile_pixbuf_t**)lua_touserdata(L, 1);
  delete pixbuf;
  return 0;
}

int tile_pixbuf_t_new_from_file(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isstring(L, 1));
  
  tile_pixbuf_t** pixbufp = (tile_pixbuf_t**)lua_newuserdata(L, sizeof(tile_pixbuf_t*));
  *pixbufp = new tile_pixbuf_t(lua_tostring(L, 1));
  lua_getfield(L, LUA_REGISTRYINDEX, "tile_pixbuf_t");
  lua_setmetatable(L, -2);
  return 1;
}

int tile_pixbuf_t_new_from_file_at_scale(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isstring(L, 1));
  
  tile_pixbuf_t** pixbufp = (tile_pixbuf_t**)lua_newuserdata(L, sizeof(tile_pixbuf_t*));
  *pixbufp = new tile_pixbuf_t(lua_tostring(L, 1), lua_tointeger(L, 2), lua_tointeger(L, 3), lua_toboolean(L, 4));
  lua_getfield(L, LUA_REGISTRYINDEX, "tile_pixbuf_t");
  lua_setmetatable(L, -2);
  return 1;
}

int tile_pixbuf_t_width_get(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  luaL_checkudata(L, 1, "tile_pixbuf_t");

  tile_pixbuf_t* pixbuf = *(tile_pixbuf_t**)lua_touserdata(L, 1);
  if (pixbuf->pixbuf)
    lua_pushinteger(L, gdk_pixbuf_get_width(pixbuf->pixbuf));
  else
    lua_pushnil(L);
  return 1;
}

int tile_pixbuf_t_height_get(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  luaL_checkudata(L, 1, "tile_pixbuf_t");

  tile_pixbuf_t* pixbuf = *(tile_pixbuf_t**)lua_touserdata(L, 1);
  if (pixbuf->pixbuf)
    lua_pushinteger(L, gdk_pixbuf_get_height(pixbuf->pixbuf));
  else
    lua_pushnil(L);
  return 1;
}

int tile_pixbuf_t_scale(lua_State *L)
{
  assert(lua_gettop(L) == 3);
  luaL_checkudata(L, 1, "tile_pixbuf_t");
  tile_pixbuf_t* pixbuf = *(tile_pixbuf_t**)lua_touserdata(L, 1);
  tile_pixbuf_t** pixbufp = (tile_pixbuf_t**)lua_newuserdata(L, sizeof(tile_pixbuf_t*));

  uint32_t width = lua_tonumber(L, 2);
  uint32_t height = lua_tonumber(L, 3);

  *pixbufp = pixbuf->scale(width, height);
  lua_getfield(L, LUA_REGISTRYINDEX, "tile_pixbuf_t");
  lua_setmetatable(L, -2);
  return 1;
}

int luaopen_tile_pixbuf_t(lua_State *L)
{
  /* --------------------
     Create Pixbuf metatable
     -------------------- */

  //  lua_newtable(L);
  luaL_newmetatable(L, "tile_pixbuf_t");
  lua_pushcclosure(L, tile_pixbuf_t___tostring, 0);  lua_setfield(L, -2, "__tostring");
  lua_pushcclosure(L, tile_pixbuf_t___gc, 0);  lua_setfield(L, -2, "__gc");

  /* --------------------
     methods table
     -------------------- */

  lua_newtable(L);
  lua_pushcclosure(L, tile_pixbuf_t_scale, 0);  lua_setfield(L, -2, "scale");

  /* --------------------
     getters/setters table
     -------------------- */
  
  lua_newtable(L);
  lua_pushcclosure(L, tile_pixbuf_t_width_get, 0); lua_setfield(L, -2, "width");
  lua_pushcclosure(L, tile_pixbuf_t_height_get, 0); lua_setfield(L, -2, "height");
  lua_pushcclosure(L, tile_pixbuf_t___index, 2); lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
  
  //  lua_setfield(L, LUA_REGISTRYINDEX, "tile_pixbuf_t");
  
  /* --------------------
     Create global tile_pixbuf_t table
     -------------------- */

  lua_pushglobaltable(L);
  lua_newtable(L);
  lua_pushcclosure(L, tile_pixbuf_t_new_from_file, 0);
  lua_setfield(L, -2, "new_from_file");
  lua_pushcclosure(L, tile_pixbuf_t_new_from_file_at_scale, 0);
  lua_setfield(L, -2, "new_from_file_at_scale");
  lua_setfield(L, -2, "tile_pixbuf_t");
  return 1;
}


