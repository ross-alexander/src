/* -*- mode: c; -*- */
%{
#include <stdio.h>
#include "common.h"
#include "parse.tab.h"
%}

%%
[0-9]+   { yylval.i = strtol(yytext, 0, 10); return INTEGER; }
"+"      { return PLUS; }
"-"      { return MINUS; }
"*"      { return TIMES; }
"⚄"	 { return DIE; }
"d"|"D"  { return DIE; }
"("      { return LPAREN; }
")"      { return RPAREN; }
"["      { return LSQUARE; }
"]"      { return RSQUARE; }
[A-Za-z][A-Za-z]* { yylval.ident = strdup(yytext); return IDENT; }
" "      { };
%%
int yywrap()
{
  return 1;
}

int yyerror(eval_t **e, const char *error)
{
fprintf(stderr, "YY %s.\n", error);
exit(1);
}
