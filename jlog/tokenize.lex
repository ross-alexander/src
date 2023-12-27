%{
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <iostream>

#include <lua.hpp>

#include "common.h"
#include "parse.tab.h"

static collector *cl;

%}

%start N S

%%

<N>[0-9]{1,4}"."[0-9]{1,4}"."[0-9]{1,4}"."[0-9]{1,4} { yylval.i = cl->push(yytext); return IPV4; }
<N>[0-9]{4}"-"[0-9][0-9]"-"[0-9][0-9]"T"[0-9]{2}":"[0-9]{2}":"[0-9]{2}("."[0-9]+)?("+"[0-9]+":"[0-9]+)? {yylval.i = cl->pushtv(yytext); return TS; }
<N>[A-Za-z_][A-Za-z0-9_@.-]* { yylval.i = cl->push(yytext); return IDENT; }
<N>[0-9]+ { yylval.i = cl->push(strtol(yytext, 0, 10)); return INTEGER; }
<N>\n { cl->linenum++; return EOL; }
<N>\" { BEGIN(S); }
<S>\" { BEGIN(N); yytext[yyleng-1]='\0'; yylval.i = cl->push(yytext); return STRING; }
<S>.  { yymore(); }

<N>"-" { return HYPHEN; }
<N>"." { return PERIOD; }
<N>":" { return COLON; }
<N>"[" { return LSQUARE; }
<N>"]" { return RSQUARE; }
<N>"=" { return ASSIGN; }
<N>" " { }
%%
void yystart(FILE *stream, collector *c)
{
  cl = c;
  yyrestart(stream);
  BEGIN(N);
}

int yywrap()
{
return 1;
}
