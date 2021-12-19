%{
  #include "eval.h"
  #include "parse.tab.h"
%}

%%

"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }
("+"|"-")?[0-9]+ { yylval.i = strtol(yytext, 0, 10); return INTEGER; }
("+"|"-")?[0-9]+\. { yylval.d = strtod(yytext, 0); return FLOAT; }
("+"|"-")?[0-9]+\.[0-9]+ { yylval.d = strtod(yytext, 0); return FLOAT; }
("+"|"-")?\.[0-9]+ { yylval.d = strtod(yytext, 0); return FLOAT; }
"(" { return LEFT_PAREN;}
")" { return RIGHT_PAREN;}
"\n" { return NEWLINE; }
. ;
