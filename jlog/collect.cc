#include <assert.h>

#include <string>
#include <map>
#include <iostream>

#include <lua.hpp>

#ifdef WITH_JS
#include <jsoncons/json.hpp>
using jsoncons::json;
#endif

#include "common.h"

collect::collect()
{
  cl = 0;
#ifdef WITH_JS
  js = 0;
#endif
}

collect::collect(collector *clp)
{
  cl = clp;
#ifdef WITH_JS
  js = 0;
#endif
  lua_newtable(cl->L);
}

void collect::kv(std::string k, std::string v)
{
#ifdef WITH_JS
  if (js) (*js)[k] = v;
#endif

  if (cl)
    {
      int top = lua_gettop(cl->L);
      assert(lua_istable(cl->L, top));
      lua_pushstring(cl->L, v.c_str());
      lua_setfield(cl->L, top, k.c_str());
    }
}


void collect::kv(std::string k, long v)
{
#ifdef WITH_JS
  if (js) (*js)[k] = v;
#endif

  if (cl)
    {
      int top = lua_gettop(cl->L);
      assert(lua_istable(cl->L, top));
      lua_pushinteger(cl->L, v);
      lua_setfield(cl->L, top, k.c_str());
    }
}

void collect::kv(std::string k, collect &c)
{
#ifdef WITH_JS
  if (js) (*js)[k] = c.js;
#endif

  if (cl)
    {
      int top = lua_gettop(cl->L);
      assert(lua_istable(cl->L, top));
      lua_pushvalue(cl->L, top-1);
      //      printf("%d %s\n", top, k.c_str());
      lua_setfield(cl->L, top, k.c_str());
      lua_remove(cl->L, top-1);
    }
}


void collect::dump()
{
#ifdef WITH_JS
  if (js) std::cout << pretty_print(*js) << '\n';
#endif

  if (cl)
    {
      int top = lua_gettop(cl->L);
      if (lua_istable(cl->L, lua_gettop(cl->L)))
	{
	  if (lua_getglobal(cl->L, "dump") == LUA_TFUNCTION)
	    {
	      lua_pushvalue(cl->L, top);
	      lua_call(cl->L, 1, 0);
	    }
	  else
	    {
	      lua_pop(cl->L, 1);
	    }
	}
    }
}
