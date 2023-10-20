#include <iostream>
#include <filesystem>
#include <lua.hpp>
#include <stdio.h>

struct path_t {
  std::filesystem::path path;
};

int path_t___tostring(lua_State *L)
{
  path_t **pp = (path_t**)luaL_checkudata(L, 1, "path_t");
  path_t *path = *pp;
  lua_pushstring(L, path->path.c_str());
  return 1;
}

int path_t___filename(lua_State *L)
{
  path_t **pp = (path_t**)luaL_checkudata(L, 1, "path_t");
  path_t *path = *pp;
  path_t **file_pp = (path_t**)lua_newuserdata(L, sizeof(path_t*));
  lua_getfield(L, LUA_REGISTRYINDEX, "path_t");
  lua_setmetatable(L, -2);
  path_t *file_p = *file_pp = new path_t;
  file_p->path = path->path.filename();
  return 1;
}


int path_t_new(lua_State *L)
{
  const char *s = luaL_checkstring(L, 1);
  path_t** pp = (path_t**)lua_newuserdata(L, sizeof(path_t*));
  lua_getfield(L, LUA_REGISTRYINDEX, "path_t");
  lua_setmetatable(L, -2);
  path_t *path = *pp = new path_t;
  path->path = s;
  return 1;
}

int path_t___add(lua_State *L)
{
  path_t *a = *((path_t**)luaL_checkudata(L, 1, "path_t"));
  path_t *b = *((path_t**)luaL_checkudata(L, 2, "path_t"));

  path_t *c = new path_t;
  c->path = a->path;
  c->path /= b->path;

  path_t** pp = (path_t**)lua_newuserdata(L, sizeof(path_t*));
  lua_getfield(L, LUA_REGISTRYINDEX, "path_t");
  lua_setmetatable(L, -2);
  *pp = c;
  return 1;
}

void path_t_init(lua_State *L)
{
  /* --------------------
     path_t
     -------------------- */

  luaL_newmetatable(L, "path_t");
  lua_pushcclosure(L, path_t___tostring, 0); lua_setfield(L, -2, "__tostring");
  lua_pushcclosure(L, path_t___add, 0); lua_setfield(L, -2, "__add");

  lua_newtable(L);
  lua_pushcclosure(L, path_t___filename, 0); lua_setfield(L, -2, "filename");
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  lua_pushglobaltable(L);
  lua_newtable(L);
  lua_pushcclosure(L, path_t_new, 0); lua_setfield(L, -2, "new");
  lua_setfield(L, -2, "path_t");
  lua_pop(L, 1);
}
