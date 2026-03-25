#include <iostream>
#include <array>
#include <vector>
#include <map>

#include <stdio.h>
#include <time.h>

#include <lua.hpp>

#include "common.h"
#include "parse.tab.h"
#include "lex.yy.h"

extern int yylex();

eval_t *f_repeat(roller_t* roller, list_t *params)
{
  assert(params->list.size() > 1);
  integer_t* count_e = dynamic_cast<integer_t*>(params->list[0]->eval_f(roller));
  assert(count_e);
  int count = count_e->value;
  list_t *res = new list_t();
  for (int i = 0; i < count; i++)
    {
      eval_t *e = params->list[1]->eval_f(roller);
      res->append(e);
    }
  return res;
}

eval_t *f_floor(roller_t* roller, list_t* params)
{
  list_t *res = new list_t;
  int value;
  if (params->list.size() > 0)
    {
      eval_t *e = params->list[0]->eval_f(roller);
      if (integer_t* i  = dynamic_cast<integer_t*>(e))
	{
	  value = i->value;
	}
      else
	{
	  assert(dynamic_cast<integer_t*>(e) != nullptr);
	}
      if (value < 1)
	value = 1;
      return new integer_t(value);
    }
  return nullptr;
}


eval_t *f_die(roller_t* roller, list_t* params)
{
  if (params->list.size() > 0)
    {
      int count = dynamic_cast<integer_t*>(params->list[0])->value;
      int die = dynamic_cast<integer_t*>(params->list[1])->value;
      int rollup = dynamic_cast<integer_t*>(params->list[2])->value;
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
      return new integer_t(sum);
    }
  return nullptr;
}

eval_t *f_plus(roller_t* roller, list_t *params)
{
  eval_t *a_e = params->list[0]->eval_f(roller);
  eval_t *b_e = params->list[1]->eval_f(roller);
  
  assert(dynamic_cast<integer_t*>(a_e) != nullptr);
  assert(dynamic_cast<integer_t*>(b_e) != nullptr);
  
  int a = dynamic_cast<integer_t*>(a_e)->value;
  int b = dynamic_cast<integer_t*>(b_e)->value;
  return new integer_t(a + b);
}

eval_t *f_minus(roller_t* roller, list_t *params)
{
  eval_t *a_e = params->list[0]->eval_f(roller);
  eval_t *b_e = params->list[1]->eval_f(roller);
  
  assert(dynamic_cast<integer_t*>(a_e) != nullptr);
  assert(dynamic_cast<integer_t*>(b_e) != nullptr);
  
  int a = dynamic_cast<integer_t*>(a_e)->value;
  int b = dynamic_cast<integer_t*>(b_e)->value;

  return new integer_t(a - b);
}

eval_t *f_times(roller_t* roller, list_t *params)
{
  eval_t *a_e = params->list[0]->eval_f(roller);
  eval_t *b_e = params->list[1]->eval_f(roller);
  
  assert(dynamic_cast<integer_t*>(a_e) != nullptr);
  assert(dynamic_cast<integer_t*>(b_e) != nullptr);

  int a = dynamic_cast<integer_t*>(a_e)->value;
  int b = dynamic_cast<integer_t*>(b_e)->value;

  return new integer_t(a * b);
}

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
  std::cout << "die(" << sum << ")\n";
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

  int start = lua_gettop(L);
  
  for (int i = 0; i < count; i++)
    {
      std::cout << "++ " << lua_gettop(L) << "\n";
      eval_t* e = (eval_t*)lua_touserdata(L, 2);
      e->eval_l(L);
      std::cout << "-- " << lua_gettop(L) << "\n";
    }
  return lua_gettop(L) - start;
}
  

/* ----------------------------------------------------------------------
   --
   -- main
   --
   ---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  srand(time(0));

  roller_t roller;
  
  roller.fmap["__plus"] = f_plus;
  roller.fmap["__minus"] = f_minus;
  roller.fmap["__times"] = f_times;
  roller.fmap["__floor"] = f_floor;
  roller.fmap["die"] = f_die;
  roller.fmap["repeat"] = f_repeat;

  lua_State *L = roller.lua = luaL_newstate();
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
  lua_pushlightuserdata(L, &roller);
  lua_setfield(L, -2, "roller");
  lua_setfield(L, -2, "__roller");
  lua_pop(L, 1);
  
  for (int i = 1; i < argc; i++)
    { 
      const char *s = argv[i];
      eval_t *e;
      yy_scan_string(s);
      yyparse(&e);
      e->dump();
      std::cout << "\n";
      eval_t* res = e->eval_f(&roller);
      res->dump();
      std::cout << "\n";
      //      std::cout << " = " << e->eval() << "\n";
      e->eval_l(L);
      for (unsigned int i = 1; i <= lua_gettop(roller.lua); i++)
	{
	  switch(lua_type(L, i))
	    {
	    case LUA_TNUMBER:
	      if (lua_isinteger(L, i))
		printf("%02d: %ld\n", i, lua_tointeger(L, i));
	      else
		printf("%02d: %f\n", i, lua_tonumber(L, i));
	      break;
	    } 
	}
    }
  return(0);
}
