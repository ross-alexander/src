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

/* ----------------------------------------------------------------------
   --
   -- C to lua (and back) functions
   --
   ---------------------------------------------------------------------- */

static Date *pushDate(lua_State *L)
{
  Date *d = (Date*)lua_newuserdata(L, sizeof(Date));
  luaL_getmetatable(L, "Date");
  lua_setmetatable(L, -2);
  return d;
}

static Date *toDate(lua_State *L, int index)
{
  Date *d = (Date *)luaL_checkudata(L, index, "Date");
  if (d == NULL) luaL_typerror(L, index, "Date");
  return d;
}

/* ----------------------------------------------------------------------
   --
   -- metamethods
   --
   ---------------------------------------------------------------------- */

static int Date___tostring(lua_State *L)
{
  char buffer[1024];
  strftime(buffer, 1024, "%c", &(toDate(L, 1)->tm));
  lua_pushfstring(L, "Date(%s)", buffer);
  return 1;
}

static int Date___add(lua_State *L)
{
  Date *d = toDate(L, 1);
  int a = lua_tointeger(L, 2);
  Date *r = pushDate(L);
  r->tick = d->tick + a * 86400;
  localtime_r(&r->tick, &r->tm);
  return 1;
}

static int Date___sub(lua_State *L)
{
  Date *d = toDate(L, 1);
  int a = lua_tointeger(L, 2);
  Date *r = pushDate(L);
  r->tick = d->tick - a * 86400;
  localtime_r(&r->tick, &r->tm);
  return 1;
}

static int Date___index(lua_State *L)
{
  assert(lua_gettop(L) == 2);
  assert(luaL_checkudata(L, 1, "Date"));
  const char* key = luaL_checkstring(L, 2);

  /* methods table */
  
  lua_pushvalue(L, lua_upvalueindex(1));
  lua_getfield(L, -1, key);
  if(!lua_isnil(L, -1))
    return 1;

  /* getter table */
  
  lua_pushvalue(L, lua_upvalueindex(2));
  lua_getfield(L, -1, key);
  
  if (!lua_isnil(L, -1))
    {
      lua_pushvalue(L, 1);
      lua_call(L, 1, 1);
    }
  return 1;
}

/* ----------------------------------------------------------------------
 *
 * class methods
 *
 ---------------------------------------------------------------------- */


static int lua_parse_date(lua_State *L)
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
  assert(lua_isinteger(L, -1));

  pstate p;
  p.year = lua_tointeger(L, -1);

  /* --------------------
     Make sure passed value is a string
     -------------------- */

  assert(lua_isstring(L, -2));
  parse_date(&p, (char*)lua_tostring(L, -2));

  /* --------------------
     Return result as a number on the stack
     -------------------- */

  lua_pushinteger(L, p.res);
  return 1;
}



static int Date_new(lua_State *L)
{
  lua_parse_date(L);
  int a = lua_tointeger(L, -1);
  Date *d = pushDate(L);
  d->tick = (time_t)(a);
  localtime_r(&d->tick, &d->tm);
  return 1;
}

static int Date_now(lua_State *L)
{
  Date *d = pushDate(L);
  d->tick = (time_t)(time(0));
  localtime_r(&d->tick, &d->tm);
  return 1;
}

/* ----------------------------------------------------------------------
   --
   -- instance methods
   --
   ---------------------------------------------------------------------- */

static int Date_wday_get(lua_State *L)
{
  Date *d = toDate(L, 1);
  int a = d->tm.tm_wday;
  lua_pushinteger(L, a);
  return 1;
}

static int Date_mday_get(lua_State *L)
{
  Date *d = toDate(L, 1);
  int a = d->tm.tm_mday;
  lua_pushinteger(L, a);
  return 1;
}

static int Date_mon_get(lua_State *L)
{
  Date *d = toDate(L, 1);
  int a = d->tm.tm_mon;
  lua_pushinteger(L, a);
  return 1;
}

static int Date_year_get(lua_State *L)
{
  Date *d = toDate(L, 1);
  int a = d->tm.tm_year;
  lua_pushinteger(L, a);
  return 1;
}


static int Date_strftime(lua_State *L)
{
  assert(lua_isstring(L, 2));
  Date *d = toDate(L, 1);
  localtime_r(&d->tick, &d->tm);
  char buffer[1024];
  strftime(buffer, sizeof(buffer), lua_tostring(L, 2), &d->tm);
  lua_pushstring(L, buffer);
  return 1;
}

/* ----------------------------------------------------------------------
 * 
 * lalL_Reg tables
 *
 ---------------------------------------------------------------------- */

static const luaL_Reg Date_class_methods[] = {
  {"new",	Date_new},
  {"now",	Date_now},
  {0, 0}
};

static const luaL_Reg Date_instance_methods[] = {
  {"strftime",	Date_strftime},
  {0, 0}
};

static const luaL_Reg Date_instance_getters[] = {
  {"wday", Date_wday_get},
  {"mday", Date_mday_get},
  {"mon",  Date_mon_get},
  {"year", Date_year_get},
  {0, 0}
};
  
static const luaL_Reg Date_meta[] = {
  {"__tostring", Date___tostring},
  {"__add", Date___add},
  {"__sub", Date___sub},
  {0, 0}
};


int luaopen_date(lua_State *L)
{
  /* if called as a dynamic library then two strings a put onto the
     stack, the module name and the module path */
  
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
  lua_pushinteger(L, year);
  lua_setfield(L, -2, "year");
  lua_pop(L, 1);
#else
  lua_pushnumber(L, year);
  lua_setfield(L, LUA_REGISTRYINDEX, "year");
#endif

  
  /* --------------------
     Add Date class
     -------------------- */

  lua_newtable(L);
  luaL_setfuncs(L, Date_class_methods, 0);
  lua_setglobal(L, "Date");            /* Set global with class methods */

  /* --------------------
     Add class methods
     -------------------- */
     
  luaL_newmetatable(L, "Date");          /* create metatable for Date, and add it to the Lua registry */
  luaL_setfuncs(L, Date_meta, 0);
  
  lua_newtable(L);
  luaL_setfuncs(L, Date_instance_methods, 0);

  lua_newtable(L);
  luaL_setfuncs(L, Date_instance_getters, 0);

  lua_pushcclosure(L, Date___index, 2); lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  // lua_pushliteral(L, "__index");
  //  lua_rawset(L, -3);                  /* metatable.__index = methods */

  //  lua_pushliteral(L, "__metatable");
  //  lua_pushvalue(L, -3);               /* dup methods table*/
  //  lua_rawset(L, -3);                  /* hide metatable:
  //                                         metatable.__metatable = methods */
  // lua_pop(L, 1);                      /* drop metatable */
  //  lua_pop(L, 1);

  return 1;
}
