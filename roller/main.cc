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

/* ----------------------------------------------------------------------
   --
   -- functions
   --
   ---------------------------------------------------------------------- */

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
   -- roller_t
   --
   ---------------------------------------------------------------------- */

roller_t::roller_t()
{
  debuglevel = 0;
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

  lua_State *L = roller_lua_init(&roller);
  
  for (int i = 1; i < argc; i++)
    { 
      const char *s = argv[i];
      eval_t *e;
      yy_scan_string(s);
      yyparse(&e);
      eval_t* res = e->eval_f(&roller);
      e->dump();
      std::cout << " = ";
      res->dump();
      std::cout << "\n";
      e->eval_l(L);
      for (unsigned int i = 1; i <= lua_gettop(L); i++)
	{
	  roller_lua_dump(L, i);
	  std::cout << "\n";
	}
    }
  return(0);
}
