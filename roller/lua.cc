#include <assert.h>
#include <lua.hpp>

#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "common.h"


/* ----------------------------------------------------------------------
   --
   -- lua
   --
   ---------------------------------------------------------------------- */

int roller_lua_tointeger(lua_State *L, int index)
{
  assert(lua_gettop(L) > 0);

  lua_getglobal(L, "__roller");
  lua_getfield(L, -1, "roller");
  roller_t *roller = (roller_t*)lua_touserdata(L, -1);
  lua_pop(L, 2);

  if (lua_isinteger(L, index))
    {
      return lua_tointeger(L, index);
    }
  else if (lua_islightuserdata(L, index))
    {
      eval_t *e = (eval_t*)lua_touserdata(L, index);
      e->eval_l(L);
      if (lua_isinteger(L, -1))
	return lua_tointeger(L, -1);
      else
	abort();
    }
  else
    abort();
  return 0;
}

int roller_lua_die(lua_State *L)
{
  assert(lua_gettop(L) >= 3);

  int count = roller_lua_tointeger(L, 1);
  int die = roller_lua_tointeger(L, 2);
  int rollup = roller_lua_tointeger(L, 3);

  int sum = 0;
  
  for (int i = 0; i < count; i++)
    {
      int r;
      do {
	r = (random() % die) + 1;
	sum += r;
      }
      while ((r == die) && rollup);
    }
  lua_pushinteger(L, sum);
  return 1;
}

int roller_lua_plus(lua_State *L)
{
  assert(lua_gettop(L) >= 2);
  int a = roller_lua_tointeger(L, 1);
  int b = roller_lua_tointeger(L, 2);
  lua_pushinteger(L, a + b);
  return 1;
}

int roller_lua_minus(lua_State *L)
{
  assert(lua_gettop(L) >= 2);
  int a = roller_lua_tointeger(L, 1);
  int b = roller_lua_tointeger(L, 2);
  lua_pushinteger(L, a - b);
  return 1;
}

int roller_lua_floor(lua_State *L)
{
  assert(lua_gettop(L) >= 1);
  int a = roller_lua_tointeger(L, 1);
  if (a < 1)
    a = 1;
  lua_pushinteger(L, a);
  return 1;
}


int roller_lua_repeat(lua_State *L)
{
  assert(lua_gettop(L) >= 2);

  int count = roller_lua_tointeger(L, 1);
  assert(lua_isuserdata(L, 2));

  lua_newtable(L);
  
  int table_index = lua_gettop(L);
  int index = 0;
  
  for (int i = 0; i < count; i++)
    {
      eval_t* e = (eval_t*)lua_touserdata(L, 2);
      int start = lua_gettop(L);
      e->eval_l(L);
      int end = lua_gettop(L);
      // printf("start = %d end = %d\n", start, end);
      for (int j = 0; j < end-start; j++)
	{
	  // printf("Setting index %d\n", index+end-start+j);
	  lua_seti(L, table_index, index+end-start+j);
	}
      index += end - start;
    }
  return 1; // lua_gettop(L) - table_index + 1;
}

void roller_lua_dump(lua_State *L, int i)
{
  switch(lua_type(L, i))
    {
    case LUA_TNUMBER:
      if (lua_isinteger(L, i))
	printf("%ld", lua_tointeger(L, i));
      else
	printf("%f", lua_tonumber(L, i));
      break;
    case LUA_TTABLE:
      {
	lua_pushnil(L);
	while (lua_next(L, i) != 0) /* pushes key & value onto stack */
	  {
	    roller_lua_dump(L, lua_gettop(L));
	    std::cout << " ";
	    /* removes 'value'; keeps 'key' for next iteration */
	    lua_pop(L, 1);
	  }
      }
      break;
    }
}

lua_State *roller_lua_init(roller_t *roller)
{
  lua_State *L = luaL_newstate();
  luaL_openselectedlibs(L, LUA_GLIBK|LUA_IOLIBK, 0);

  const luaL_Reg lua_functions[] = {
    {"__plus",			roller_lua_plus},
    {"__minus",			roller_lua_minus},
    {"__floor",			roller_lua_floor},
    {"die",			roller_lua_die},
    {"repeat",			roller_lua_repeat},
    {0, 0}
  };
  lua_pushglobaltable(L);
  lua_newtable(L);
  luaL_newlib(L, lua_functions);
  lua_setfield(L, -2, "functions");
  lua_pushlightuserdata(L, roller);
  lua_setfield(L, -2, "roller");
  lua_setfield(L, -2, "__roller");
  lua_pop(L, 1);
  return L;
}
