/* ----------------------------------------------------------------------
   --
   -- test can load tile functions but really client.c does all of this
   --
   ---------------------------------------------------------------------- */

#include <stdio.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

extern int luaopen_tile(lua_State*);

int main(int argc, char *argv[])
{
  lua_State *luastate = luaL_newstate();
  luaL_openselectedlibs(luastate, LUA_GLIBK|LUA_IOLIBK, 0);
  luaL_requiref(luastate, "tile", luaopen_tile, 0);
}
