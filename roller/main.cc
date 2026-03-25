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

eval_t *f_floor(roller_t* roller, list_t* params)
{
  list_t *res = new list_t;
  if (params->list.size() > 0)
    {
      int value = dynamic_cast<integer_t*>(params->list[0])->value;
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

      std::cout << "die(" << count << ", " << die << ", " << rollup << ")\n";
      
      for (int i = 0; i < count; i++)
	{
	  int r;
	  do {
	    r = (random() % die) + 1;
	    sum += r;
	    std::cout << "Rolling " << r << "\n";
	  }
	  while ((r == die) && rollup);
	}
      return new integer_t(sum);
    }
  return nullptr;
}

eval_t *f_plus(roller_t* roller, list_t *params)
{
  assert(dynamic_cast<integer_t*>(params->list[0]) != nullptr);
  assert(dynamic_cast<integer_t*>(params->list[1]) != nullptr);
  
  int a = dynamic_cast<integer_t*>(params->list[0])->value;
  int b = dynamic_cast<integer_t*>(params->list[1])->value;
  return new integer_t(a + b);
}

eval_t *f_minus(roller_t* roller, list_t *params)
{
  assert(dynamic_cast<integer_t*>(params->list[0]) != nullptr);
  assert(dynamic_cast<integer_t*>(params->list[1]) != nullptr);

  int a = dynamic_cast<integer_t*>(params->list[0])->value;
  int b = dynamic_cast<integer_t*>(params->list[1])->value;
  return new integer_t(a - b);
}

eval_t *f_times(roller_t* roller, list_t *params)
{
  std::cout << "times ";
  params->dump();
  std::cout << "\n";
  
  int a = dynamic_cast<integer_t*>(params->list[0])->value;
  int b = dynamic_cast<integer_t*>(params->list[1])->value;
  return new integer_t(a * b);
}

int main(int argc, char *argv[])
{
  srand(time(0));

  roller_t roller;
  
  roller.fmap["die"] = f_die;
  roller.fmap["__plus"] = f_plus;
  roller.fmap["__minus"] = f_minus;
  roller.fmap["__times"] = f_times;
  roller.fmap["__floor"] = f_floor;
  
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
    }
  return(0);
}
