#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "eval.h"
#include "parse.tab.h"
#include "tokenize.h"

/* ----------------------------------------------------------------------
--
-- lex helper functions
--
---------------------------------------------------------------------- */

void yyerror(eval_t *eval, const char *s)
{
  fprintf(stderr, "parse error on line %d\n", eval->line_no);
  exit(1);
}

int yywrap()
{
  return 1;
}


/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
      fprintf(stderr, "%s: <filename>\n", argv[0]);
      exit(1);
    }
  FILE *istream;

  if ((istream = fopen(argv[1], "r")) == 0)
    {
      fprintf(stderr, "%s: error reading %s : %s\n", argv[0], argv[1], strerror(errno));
      exit(1);
    }

  eval_t eval;
  eval.line_no = 1;
  yydebug = 0;
  yyrestart(istream);
  yyparse(&eval);
  return 0;
}
