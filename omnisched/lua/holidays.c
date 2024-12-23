#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "common.h"

extern int luaopen_date(lua_State*);

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  pstate g;
  time_t now = time(0);

  /* --------------------
     Get current year
     -------------------- */

  struct tm now_tm;
  gmtime_r(&now, &now_tm);

  g.year = now_tm.tm_year + 1900;

  /* --------------------
     Open lua libraries
     -------------------- */

  lua_State *L = luaL_newstate();   /* opens Lua */
  luaL_openlibs(L);
  luaopen_date(L);
  
  /* --------------------
     Add global gloop to the registry
     -------------------- */

  lua_pushstring(L, "gloop");
  lua_pushlightuserdata(L, &g);
  lua_settable(L, LUA_REGISTRYINDEX);

  if(argc > 1)
    {
      int ret = luaL_dofile(L, argv[1]);
      assert(ret == 0);
    }
  lua_close(L);
  return 0;
}
