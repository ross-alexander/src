#include <iostream>
#include <array>
#include <vector>
#include <map>
#include <string>

#include <assert.h>

#include <lua.hpp>

#include "common.h"

/* ----------------------------------------------------------------------
   --
   -- integer
   --
   ---------------------------------------------------------------------- */

integer_t::integer_t(int i)
{
  value = i;
}

eval_t* integer_t::eval_f(roller_t* roller)
{
  return new integer_t(value);
}

int integer_t::eval_l(lua_State *L)
{
  lua_pushinteger(L, value);
  return true;
}

void integer_t::dump()
{
  std::cout << value;
}

/* ----------------------------------------------------------------------
   --
   -- func
   --
   ---------------------------------------------------------------------- */


func_t::func_t(char *f, list_t *p)
{
  function = f;
  params = p;
}

eval_t *func_t::eval_f(roller_t *roller)
{
  if (!roller->fmap.count(this->function))
    {
      std::cerr << "Function " << this->function << " not found.\n";
      exit(1);
    }

  std::cout << "eval " << this->function << " ";
  this->params->dump();
  std::cout << "\n";

  // Do not evaluate parameters first as we want lazy evaulation
  
  eval_t *res = (*roller->fmap[this->function])(roller, params);
  return res;
}


int func_t::eval_l(lua_State *L)
{
  lua_getglobal(L, "__roller");
  assert(lua_istable(L, -1));

  lua_getfield(L, -1, "functions");
  lua_getfield(L, -1, function.c_str());
  lua_remove(L, -2); // remove functions table
  lua_remove(L, -2); // remove roller table
  if (lua_isfunction(L, -1))
    {
      std::cout << "Found lua function " << function << "\n";
      unsigned int f_index = lua_gettop(L);
      for (auto p : params->list)
	{
	  lua_pushlightuserdata(L, p);
	}
      int nargs = lua_gettop(L) - f_index;
      std::cout << "** " << nargs << "\n";
      lua_call(L, nargs, LUA_MULTRET);
    }
  return true;
}

void func_t::dump()
{
  std::cout << function << "(";
  params->dump();
  std::cout << ")";
}

/* ----------------------------------------------------------------------
   --
   -- list
   --
   ---------------------------------------------------------------------- */


list_t::list_t()
{
}

eval_t* list_t::eval_f(roller_t *roller)
{
  list_t *res = new list_t;
  for (auto i : list)
    {
      res->append(i->eval_f(roller));
    }
  return res;
}

int list_t::eval_l(lua_State *L)
{
  for (auto i : list)
    {
      i->eval_l(L);
    }
  return true;
}

void list_t::dump()
{
  int c = 0;
  std::cout << "(";
  for (auto i : list)
    {
      if (c > 0)
	std::cout << ", ";
      i->dump();
      c++;
    }
  std::cout << ")";
}

void list_t::append(eval_t *e)
{
  list.push_back(e);
}
