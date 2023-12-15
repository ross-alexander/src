#include <iostream>
#include <array>
#include <vector>
#include <map>

#include <stdio.h>
#include <time.h>

#include "common.h"
#include "parse.tab.h"
#include "lex.yy.h"

extern int yylex();

int main(int argc, char *argv[])
{
  srand(time(0));

  if (argc > 1)
    { 
      const char *s = argv[1];
      eval_t *e;
      yy_scan_string(s);
      yyparse(&e);
      e->dump();
      std::cout << " = " << e->eval() << "\n";
    }
}
