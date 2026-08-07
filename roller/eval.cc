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

void integer_t::dump()
{
  std::cout << value; // << ":int";
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

  if (roller->debuglevel > 0)
    {
      std::cout << "eval " << this->function << " ";
      this->params->dump();
      std::cout << "\n";
    }
  
  // Do not evaluate parameters first as we want lazy evaulation
  
  eval_t *res = (*roller->fmap[this->function])(roller, params);
  return res;
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
