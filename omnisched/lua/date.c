#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "common.h"


#if LUA_VERSION_NUM > 501
int luaL_typerror (lua_State *L, int narg, const char *tname)
{
  const char *msg = lua_pushfstring(L, "%s expected, got %s", tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}
#endif

static Date *toDate (lua_State *L, int index);

/* ----------------------------------------------------------------------
--
-- lua_date
--
---------------------------------------------------------------------- */

int lua_date(lua_State *L)
{
  /* --------------------
     Get year from lua globals
     -------------------- */

#if LUA_VERSION_NUM > 501
  lua_pushglobaltable(L);
  lua_getfield(L, -1, "year");
  lua_remove(L, -2);
#else
  lua_pushstring(L, "year");
  lua_gettable(L, LUA_GLOBALSINDEX);
#endif
  assert(lua_isnumber(L, -1));

  pstate p;
  p.year = (int)lua_tonumber(L, -1);

  /* --------------------
     Make sure passed value is a string
     -------------------- */

  assert(lua_isstring(L, -2));
  parse_date(&p, (char*)lua_tostring(L, -2));

  /* --------------------
     Return result as a number on the stack
     -------------------- */

  lua_pushnumber(L, p.res);
  return 1;
}

static Date *toDate(lua_State *L, int index)
{
  Date *bar = (Date *)luaL_checkudata(L, index, "Date");
  if (bar == NULL) luaL_typerror(L, index, "Date");
  return bar;
}

static Date *pushDate(lua_State *L)
{
  Date *bar = (Date*)lua_newuserdata(L, sizeof(Date));
  luaL_getmetatable(L, "Date");
  lua_setmetatable(L, -2);
  return bar;
}

static int Date_tostring(lua_State *L)
{
  char buff[1024];
  strftime(buff, 1024, "%c", &toDate(L, 1)->tm);
  lua_pushfstring(L, "Date(%s)", buff);
  return 1;
}

static int Date_new(lua_State *L)
{
  lua_date(L);
  lua_Number a = lua_tonumber(L, -1);
  Date *bar = pushDate(L);
  //  lua_number2int(bar->tick, a);
  bar->tick = (time_t)(a);
  localtime_r(&bar->tick, &bar->tm);
  return 1;
}

static int Date_now(lua_State *L)
{
  Date *bar = pushDate(L);
  bar->tick = (time_t)(time(0));
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

/* ----------------------------------------------------------------------
 * 
 * Date_strftime
 *
 ---------------------------------------------------------------------- */

static int Date_strftime(lua_State *L)
{
  Date *bar = toDate(L, 1);
  localtime_r(&bar->tick, &bar->tm);
  assert(lua_isstring(L, 2));
  char buf[1024];
  strftime(buf, sizeof(buf), lua_tostring(L, 2), &bar->tm);
  lua_pushstring(L, buf);
  return 1;
}


static int Date_add(lua_State *L)
{
  Date *d = toDate(L, 1);
  int a;
  a = (int)(lua_tonumber(L, 2));
  Date *r = pushDate(L);
  r->tick = d->tick + a * 86400;
  localtime_r(&r->tick, &r->tm);
  return 1;
}

static int Date_sub(lua_State *L)
{
  Date *d = toDate(L, 1);
  int a = (int)(lua_tonumber(L, 2));
  Date *r = pushDate(L);
  r->tick = d->tick - a * 86400;
  localtime_r(&r->tick, &r->tm);

  return 1;
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

static const luaL_Reg Date_methods[] = {
  {"new",	Date_new},
  {"now",	Date_now},
  {"wday",	Date_wday},
  {"omni",	Date_omni_holiday},
  {"monthly",	Date_omni_monthly},
  {"strftime",	Date_strftime},
  {0, 0}
};

static const luaL_Reg Date_meta[] = {
  {"__tostring", Date_tostring},
  {"__add", Date_add},
  {"__sub", Date_sub},
  {0, 0}
};

int luaopen_date(lua_State *L)
{
  /* --------------------
     Get current year
     -------------------- */

  time_t now = time(0);
  struct tm now_tm;
  gmtime_r(&now, &now_tm);
  int year = now_tm.tm_year + 1900;

  /* --------------------
     Set the global variable year to the year parameter
     -------------------- */

#if LUA_VERSION_NUM > 501
  lua_pushglobaltable(L);
  lua_pushnumber(L, year);
  lua_setfield(L, -2, "year");
  lua_pop(L, 1);
  lua_pop(L, 1);
#else
  lua_pushnumber(L, year);
  lua_setfield(L, LUA_REGISTRYINDEX, "year");
#endif

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

  return 1;
}
