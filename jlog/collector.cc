#include <assert.h>

#include <string>
#include <map>
#include <iostream>

#include <jsoncons/json.hpp>
#include <lua.hpp>

using jsoncons::json;

#include "common.h"

collector::collector(lua_State *lua)
{
  linenum = 1;
  limit = 0;
  L = lua;
}

int collector::update(collect *c)
{
  int top = lua_gettop(L);
  if (lua_istable(L, lua_gettop(L)))
    {
      if (lua_getglobal(L, "update") == LUA_TFUNCTION)
	{
	  lua_pushvalue(L, top);
	  lua_call(L, 1, 1);
	  int ret = lua_tointeger(L, lua_gettop(L));
	  lua_pop(L, 1);
	  return ret;
	}
      else
	{
	  lua_pop(L, 2);
	}
    }
  return 0;
}

void collector::dump()
{
  if (lua_getglobal(L, "dump") == LUA_TFUNCTION)
    lua_call(L, 0, 0);
  else
    lua_pop(L, 1);
}

