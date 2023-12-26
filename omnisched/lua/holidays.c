#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "common.h"

int luaL_typerror (lua_State *L, int narg, const char *tname)
{
  const char *msg = lua_pushfstring(L, "%s expected, got %s", tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}

static Date *toDate (lua_State *L, int index);

/* ----------------------------------------------------------------------
--
-- lua functions
--
---------------------------------------------------------------------- */

int lua_date(lua_State *L)
{
  lua_pushstring(L, "gloop");
  lua_gettable(L, LUA_REGISTRYINDEX);
  assert(lua_isuserdata(L, -1));

  pstate *g = lua_touserdata(L, -1);

  lua_getglobal(L, "year");
  assert(lua_isnumber(L, -1));
  g->year = (int)lua_tonumber(L, -1);

  parse_date(g, (char*)lua_tostring(L, -3));
  lua_pushnumber(L, g->res);
  return 1;
}

int lua_omni(lua_State *L)
{
  Date *bar = toDate(L, 1);

  char tmp[512];
  strftime(tmp, 512, "%j%t%b %d\t\t# %a %d %b %Y\n", &bar->tm);
  printf("%s", tmp);
  return 0;
}


/* ----------------------------------------------------------------------
--
-- Date functions
--
---------------------------------------------------------------------- */

static Date *toDate(lua_State *L, int index)
{
  Date *bar = (Date *)luaL_checkudata(L, index, "Date");
  if (bar == NULL) luaL_typerror(L, index, "Date");
  return bar;
}

static int Date_omni_holiday(lua_State *L)
{
  char s[1024];
  strftime(s, 1024, "%j%t%b %d\t\t# %a %d %b %Y", &toDate(L, 1)->tm);
  lua_pushfstring(L, "%s", s);
  return 1;
}

static int Date_omni_monthly(lua_State *L)
{
  char s[1024];
  strftime(s, 1024, "\t-day %d -month %b", &toDate(L, 1)->tm);
  lua_pushfstring(L, "%s", s);
  return 1;
}

static int Date_tostring(lua_State *L)
{
  char buff[1024];
  strftime(buff, 1024, "%c", &toDate(L, 1)->tm);
  lua_pushfstring(L, "Date(%s)", buff);
  return 1;
}

static Date *pushDate(lua_State *L)
{
  Date *bar = (Date*)lua_newuserdata(L, sizeof(Date));
  luaL_getmetatable(L, "Date");
  lua_setmetatable(L, -2);
  return bar;
}

static int Date_new(lua_State *L)
{
  lua_date(L);
  lua_Number a = lua_tonumber(L, -1);
  Date *bar = pushDate(L);
  bar->tick = (int)a;
  localtime_r(&bar->tick, &bar->tm);
  return 1;
}

static int Date_wday(lua_State *L)
{
  Date *bar = toDate(L, 1);
  int a = bar->tm.tm_wday;
  lua_pushnumber(L, a);
  return 1;
}

static int Date_add(lua_State *L)
{
  Date *d = toDate(L, 1);
  int a = (int)lua_tonumber(L, 2);
  Date *r = pushDate(L);
  r->tick = d->tick + a * 86400;
  localtime_r(&r->tick, &r->tm);
  return 1;
}

static int Date_sub(lua_State *L)
{
  Date *d = toDate(L, 1);
  int a = (int)lua_tonumber(L, 2);
  Date *r = pushDate(L);
  r->tick = d->tick - a * 86400;
  localtime_r(&r->tick, &r->tm);

  return 1;
}

static const luaL_Reg Date_methods[] = {
  {"new",	Date_new},
  {"wday",	Date_wday},
  {"omni",	Date_omni_holiday},
  {"monthly",	Date_omni_monthly},
  {0, 0}
};

static const luaL_Reg Date_meta[] = {
  {"__tostring", Date_tostring},
  {"__add", Date_add},
  {"__sub", Date_sub},
  {0, 0}
};

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

  /* --------------------
     Set the global variable year to the year parameter
     -------------------- */

  lua_pushnumber(L, g.year);
  lua_setglobal(L, "year");

  /* --------------------
     Add global gloop to the registry
     -------------------- */

  lua_pushstring(L, "gloop");
  lua_pushlightuserdata(L, &g);
  lua_settable(L, LUA_REGISTRYINDEX);

  /* --------------------
     register function which converts string to tick
     -------------------- */

  lua_register(L, "date", lua_date);
  lua_register(L, "omni", lua_omni);

  /* --------------------
     Add Date object
     -------------------- */

  luaL_openlib(L, "Date", Date_methods, 0);  /* create methods table, add it to the globals */
  luaL_newmetatable(L, "Date");          /* create metatable for Date, and add it to the Lua registry */
  luaL_openlib(L, 0, Date_meta, 0);    /* fill metatable */
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -3);               /* dup methods table*/
  lua_rawset(L, -3);                  /* metatable.__index = methods */
  lua_pushliteral(L, "__metatable");
  lua_pushvalue(L, -3);               /* dup methods table*/
  lua_rawset(L, -3);                  /* hide metatable:
                                         metatable.__metatable = methods */
  lua_pop(L, 1);                      /* drop metatable */

  if(argc > 1)
    {
      int ret = luaL_dofile(L, argv[1]);
      assert(ret == 0);
    }
  lua_close(L);
  return 0;
}
