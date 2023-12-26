%{
#include <string.h>
#include "common.h"
#include "parse.h"
%}

%x string comment

%%
"#"		{ BEGIN(comment); }
<comment>.	;
<comment>\n	{ BEGIN(INITIAL); }
"+"		{ return PLUS; }
"-"		{ return MINUS; }
"("		{ return LPAREN; }
")"		{ return RPAREN; }
"~"		{ return TILDE; }
"!"		{ return NOT; }
":"		{ return COLON; }
[0-9]+		{ yylval.text = yytext; return INTEGER; }
day		{ return DAY; }
week		{ return WEEK; }
month		{ return MONTH; }

first		{ return FIRST; }
second		{ return SECOND; }
third	;
forth	;
fifth	;
sixth	;
seventh	;
eighth	;
nineth	;
tenth	;
last		{ return LAST; }
monday		{ return MON; }
tuesday		{ return TUE; }
wednesday	;
thursday	;
friday	;
saturday	;
sunday		;
january		{ return JAN; }
jan		{ return JAN; }
february	;
march		;
april		;
may		{ return MAY; }
june		;
july		;
august		{ return AUG; }
september	{ return SEP; }
october		;
november	;
december	{ return DEC; }
of		{ return OF; }
special		{ return SPECIAL; }
cascade		{ return CASCADE; }
\"		{ BEGIN(string); }
<string>[A-Za-z0-9 ]	{ yymore(); }
<string>\"	{ yytext[yyleng - 1] = '\0'; yylval.text = strdup(yytext); BEGIN(INITIAL); return STRING; }
[A-Za-z][A-Za-z0-9]* { yylval.text = strdup(yytext); return IDENT; }
.		;
\n		;
%%
int yywrap(void)
{
return 1;
}
