/* -*- mode: c; -*- */
%{
#include <array>
#include <vector>
#include <map>
#include <string>
  
#include <stdio.h>

#include <lua.hpp>

#include "common.h"
#include "parse.tab.h"
%}

%%
"+"      { return PLUS; }
"-"      { return MINUS; }
"*"      { return TIMES; }
"("      { return LPAREN; }
")"      { return RPAREN; }
"["      { return LSQUARE; }
"]"      { return RSQUARE; }
"{"      { return LBRACE; }
"}"      { return RBRACE; }
","      { return COMMA; }
"!"      { return BANG; }
"⚄"	 { return DIE; }
"d"|"D"  { return DIE; }
[0-9]+   { yylval.i = strtol(yytext, 0, 10); return INTEGER; }
[A-Za-z][A-Za-z]* { yylval.ident = strdup(yytext); return IDENT; }
" "      { };
%%
int yywrap()
{
  /* return 1 for end of file/string */
  return 1;
}

int yyerror(eval_t **e, const char *error)
{
  fprintf(stderr, "Parse error %s.\n", error);
  exit(1);
}
