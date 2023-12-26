%{
#include <string.h>
#include <time.h>
#include "common.h"
#include "parse.h"
%}

%x string dstring comment

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
at		{ return AT; }
first		{ return FIRST; }
second		{ return SECOND; }
third		{ return THIRD; }
fourth		{ return FORTH; }
fifth		{ return FIFTH; }
last		{ return LAST; }

monday		{ return MON; }
tuesday		{ return TUE; }
wednesday	{ return WED; }
thursday	{ return THU; }
friday		{ return FRI; }
saturday	{ return SAT; }
sunday		{ return SUN; }

january		{ return JAN; }
jan		{ return JAN; }
february	{ return FEB; }
feb		{ return FEB; }
march		{ return MAR; }
mar		{ return MAR; }
april		{ return APR; }
apr		{ return APR; }
may		{ return MAY; }
june		{ return JUN; }
jun		{ return JUN; }
july		{ return JUL; }
jul		{ return JUL; }
august		{ return AUG; }
aug		{ return AUG; }
september	{ return SEP; }
sep		{ return SEP; }
october		{ return OCT; }
oct		{ return OCT; }
november	{ return NOV; }
nov		{ return NOV; }
december	{ return DEC; }
dec		{ return DEC; }
of		{ return OF; }

\"		{ BEGIN(dstring); }
<dstring>[A-Za-z0-9 ]	{ yymore(); }
<dstring>\"	{ yytext[yyleng - 1] = '\0'; yylval.text = strdup(yytext); BEGIN(INITIAL); return STRING; }

\'		{ BEGIN(string); }
<string>[A-Za-z0-9 ]	{ yymore(); }
<string>\'	{ yytext[yyleng - 1] = '\0'; yylval.text = strdup(yytext); BEGIN(INITIAL); return STRING; }

[A-Za-z][A-Za-z0-9]* { yylval.text = strdup(yytext); return IDENT; }
.		;
\n		;
%%
int yywrap(void)
{
  return 1;
}

int parse_date(pstate* g, char *s)
{
  yy_scan_string(s);
  return yyparse(g);
}
