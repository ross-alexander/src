%{
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <iostream>

#include <jsoncons/json.hpp>
#include <lua.hpp>

using jsoncons::json;

#include "common.h"
#include "parse.tab.h"

%}

%start N S

%%

<N>[0-9]{2}":"[0-9]{2}":"[0-9]{2} { yylval.s = new std::string(yytext); return TIME; }
<N>[0-9]{1,4}"."[0-9]{1,4}"."[0-9]{1,4}"."[0-9]{1,4} { yylval.s = new std::string(yytext); return IPV4; }
<N>[0-9]{4}"-"[0-9]{2}"-"[0-9]{2}"T"[0-9]{2}":"[0-9]{2}":"[0-9]{2}"."[0-9]+(("+"[0-9]{2}":"[0-9]{2})|"Z") { yylval.s = new std::string(yytext); return TS; }
<N>[A-Za-z_][A-Za-z0-9_@.-]* { yylval.s = new std::string(yytext); return IDENT; }
<N>[0-9]+ { yylval.s = new std::string(yytext); return INTEGER; }
<N>\" { BEGIN(S); }
<S>\" { BEGIN(N); yytext[yyleng-1]='\0'; yylval.s = new std::string(yytext); return STRING; }
<S>.  { yymore(); }
<N>\n { return EOL; }
<N>"-" { return HYPHEN; }
<N>"." { return PERIOD; }
<N>":" { return COLON; }
<N>"[" { return LSQUARE; }
<N>"]" { return RSQUARE; }
<N>"=" { return ASSIGN; }
<N>" " { }
%%
void yystart(FILE *stream)
{
  yyrestart(stream);
  BEGIN(N);
}

int yywrap()
{
return 1;
}
