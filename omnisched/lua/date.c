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

static date_t* push_date_t(lua_State *L)
{
  date_t *d = (date_t*)lua_newuserdata(L, sizeof(date_t));
  luaL_getmetatable(L, "date_t");
  lua_setmetatable(L, -2);
  return d;
}

static date_t* to_date_t(lua_State *L, int index)
{
  date_t *d = (date_t*)luaL_checkudata(L, index, "date_t");
  if (d == NULL) luaL_typerror(L, index, "date_t");
  return d;
}

/* ----------------------------------------------------------------------
   --
   -- metamethods
   --
   ---------------------------------------------------------------------- */

static int date_t___tostring(lua_State *L)
{
  char buffer[1024];
  strftime(buffer, 1024, "%c", &(to_date_t(L, 1)->tm));
  lua_pushfstring(L, "Date(%s)", buffer);
  return 1;
}

static int date_t___add(lua_State *L)
{
  date_t *d = to_date_t(L, 1);
  int a = lua_tointeger(L, 2);
  date_t *r = push_date_t(L);
  r->tick = d->tick + a * 86400;
  localtime_r(&r->tick, &r->tm);
  return 1;
}

static int date_t___sub(lua_State *L)
{
  date_t *d = to_date_t(L, 1);
  int a = lua_tointeger(L, 2);
  date_t *r = push_date_t(L);
  r->tick = d->tick - a * 86400;
  localtime_r(&r->tick, &r->tm);
  return 1;
}

static int date_t___index(lua_State *L)
{
  assert(lua_gettop(L) == 2);
  assert(luaL_checkudata(L, 1, "date_t"));
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

  parse_state_t p;
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



static int date_t_new(lua_State *L)
{
  lua_parse_date(L);
  int a = lua_tointeger(L, -1);
  date_t *d = push_date_t(L);
  d->tick = (time_t)(a);
  localtime_r(&d->tick, &d->tm);
  return 1;
}

static int date_t_now(lua_State *L)
{
  date_t *d = push_date_t(L);
  d->tick = (time_t)(time(0));
  localtime_r(&d->tick, &d->tm);
  return 1;
}

/* ----------------------------------------------------------------------
   --
   -- instance methods
   --
   ---------------------------------------------------------------------- */

static int date_t_wday_get(lua_State *L)
{
  date_t *d = to_date_t(L, 1);
  int a = d->tm.tm_wday;
  lua_pushinteger(L, a);
  return 1;
}

static int date_t_mday_get(lua_State *L)
{
  date_t *d = to_date_t(L, 1);
  int a = d->tm.tm_mday;
  lua_pushinteger(L, a);
  return 1;
}

static int date_t_mon_get(lua_State *L)
{
  date_t *d = to_date_t(L, 1);
  int a = d->tm.tm_mon;
  lua_pushinteger(L, a);
  return 1;
}

static int date_t_year_get(lua_State *L)
{
  date_t *d = to_date_t(L, 1);
  int a = d->tm.tm_year;
  lua_pushinteger(L, a);
  return 1;
}


static int date_t_strftime(lua_State *L)
{
  assert(lua_isstring(L, 2));
  date_t *d = to_date_t(L, 1);
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

static const luaL_Reg date_t_class_methods[] = {
  {"new",	date_t_new},
  {"now",	date_t_now},
  {0, 0}
};

static const luaL_Reg date_t_instance_methods[] = {
  {"strftime",	date_t_strftime},
  {0, 0}
};

static const luaL_Reg date_t_instance_getters[] = {
  {"wday", date_t_wday_get},
  {"mday", date_t_mday_get},
  {"mon",  date_t_mon_get},
  {"year", date_t_year_get},
  {0, 0}
};
  
static const luaL_Reg date_t_meta[] = {
  {"__tostring", date_t___tostring},
  {"__add", date_t___add},
  {"__sub", date_t___sub},
  {0, 0}
};

/* ----------------------------------------------------------------------
   --
   -- luaopen_date
   --
   ---------------------------------------------------------------------- */

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
     Add class methods
     -------------------- */
     
  luaL_newmetatable(L, "date_t");          /* create metatable for date_t, and add it to the Lua registry */
  luaL_setfuncs(L, date_t_meta, 0);
  
  lua_newtable(L);
  luaL_setfuncs(L, date_t_instance_methods, 0);

  lua_newtable(L);
  luaL_setfuncs(L, date_t_instance_getters, 0);

  lua_pushcclosure(L, date_t___index, 2); lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  // lua_pushliteral(L, "__index");
  //  lua_rawset(L, -3);                  /* metatable.__index = methods */

  //  lua_pushliteral(L, "__metatable");
  //  lua_pushvalue(L, -3);               /* dup methods table*/
  //  lua_rawset(L, -3);                  /* hide metatable:
  //                                         metatable.__metatable = methods */
  // lua_pop(L, 1);                      /* drop metatable */
  //  lua_pop(L, 1);
  
  /* --------------------
     Add date_t class
     -------------------- */
  
  lua_newtable(L);
  luaL_setfuncs(L, date_t_class_methods, 0);
  lua_pushvalue(L, -1);
  lua_setglobal(L, "Date");            /* Set global with class methods */

  return 1;
}
