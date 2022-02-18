#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif


#ifdef HAVE_GDK30_H
#include <gdk/gdk.h>
#endif

#ifdef HAVE_XCB_XCB_H
#include <xcb/xcb.h>
#endif

#include <lua.hpp>

#include <string>

#include "tile.h"

/* ----------------------------------------------------------------------
--
-- tile_format_t
--
---------------------------------------------------------------------- */

tile_format_t::tile_format_t(std::string p)
{
  format = gdk_pixbuf_get_file_info(p.c_str(), &width, &height);
}

int tile_format_t_width_get(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  luaL_checkudata(L, 1, "tile_format_t");

  tile_format_t* format = *(tile_format_t**)lua_touserdata(L, 1);
  if (format->format)
    lua_pushinteger(L, format->width);
  else
    lua_pushnil(L);
  return 1;
}

int tile_format_t_height_get(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  luaL_checkudata(L, 1, "tile_format_t");

  tile_format_t* format = *(tile_format_t**)lua_touserdata(L, 1);
  if (format->format)
    lua_pushinteger(L, format->height);
  else
    lua_pushnil(L);
  return 1;
}

int tile_format_t_scalable_get(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  luaL_checkudata(L, 1, "tile_format_t");

  tile_format_t* format = *(tile_format_t**)lua_touserdata(L, 1);
  if (format->format)
    lua_pushinteger(L, gdk_pixbuf_format_is_scalable(format->format));
  else
    lua_pushnil(L);
  return 1;
}

int tile_format_t___index(lua_State *L)
{
  assert(lua_gettop(L) == 2);
  assert(luaL_checkudata(L, 1, "tile_format_t"));
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


int tile_format_t_get_file_info(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isstring(L, 1));
  
  tile_format_t** pixbufp = (tile_format_t**)lua_newuserdata(L, sizeof(tile_format_t*));
  *pixbufp = new tile_format_t(std::string(lua_tostring(L, 1)));
  lua_getfield(L, LUA_REGISTRYINDEX, "tile_format_t");
  lua_setmetatable(L, -2);
  return 1;
}


int luaopen_tile_format_t(lua_State *L)
{
  /* --------------------
     Create Pixbuf metatable
     -------------------- */

  luaL_newmetatable(L, "tile_format_t");
  //  lua_pushcclosure(L, tile_pixbuf_t___tostring, 0);  lua_setfield(L, -2, "__tostring");
  //  lua_pushcclosure(L, tile_pixbuf_t___gc, 0);  lua_setfield(L, -2, "__gc");

  /* --------------------
     methods table
     -------------------- */

  lua_newtable(L);
  //  lua_pushcclosure(L, tile_pixbuf_t_scale, 0);  lua_setfield(L, -2, "scale");

  /* --------------------
     getters/setters table
     -------------------- */
  
  lua_newtable(L);
  lua_pushcclosure(L, tile_format_t_width_get, 0); lua_setfield(L, -2, "width");
  lua_pushcclosure(L, tile_format_t_height_get, 0); lua_setfield(L, -2, "height");
  lua_pushcclosure(L, tile_format_t_scalable_get, 0); lua_setfield(L, -2, "scalable");
  lua_pushcclosure(L, tile_format_t___index, 2); lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  /* --------------------
     Create global tile_pixbuf_t table
     -------------------- */

  lua_pushglobaltable(L);
  lua_newtable(L);
  lua_pushcclosure(L, tile_format_t_get_file_info, 0);
  lua_setfield(L, -2, "get_file_info");
  lua_setfield(L, -2, "tile_format_t");
  return 1;
}
