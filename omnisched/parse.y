%token FIRST SECOND THIRD FORTH FIFTH SIXTH SEVENTH EIGHTH NINETH TENTH LAST
%token MON TUE WED THU FRI SAT SUN
%token JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC
%token DAY WEEK MONTH YEAR OF SPECIAL CASCADE IDENT
%token INTEGER STRING PLUS MINUS LPAREN RPAREN NOT AND OR EQ NE TILDE EMPTY FUNC LIST TICK COLON

%{
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "common.h"
%}

%union {
  tree* expr;
  action act;
  int mday;
  int wday;
  int month;
  int ordinal;
  time_t date;
  char* text;
}
%type <text> INTEGER STRING SPECIAL CASCADE IDENT
%type <mday> mday 
%type <wday> wday
%type <month> month
%type <ordinal> ordinal
%type <date> date
%type <expr> expr line lines parse
%parse-param { tree **restree }

%%

parse	: lines { tree** t = restree; *t = $1; }
	;

lines	: line { $$ = NewOp(LIST, $1, NULL); printf("New List created %p\n", $$); }
	| parse line { $$ = NewOp(LIST, $1, $2); printf("List appened %p\n", $$); }
	;

line	: date { $$ = NewOp(FUNC, "print", NewOp(FUNC, "omni", NewInt(TICK, $1))); printf("New Tick %p\n", $$); }
	| date TILDE expr
	{
	  tree *t = NewInt(TICK, $1);
	  $$ = NewOp(TILDE,  t, $3); printf("TILDE %p %p %p\n", $$, t, $3);
	}
	;

expr : IDENT LPAREN expr RPAREN { $$ = NewOp(FUNC, NewStr(IDENT, $1), $3); }
	| STRING { $$ = NewStr(STRING, $1); }
	| INTEGER { $$ = NewInt(INTEGER, strtol($1, NULL, 10)); }
	| NOT expr { $$ = NewOp(NOT, $2, NULL); }
	| expr EQ expr { $$ = NewOp(EQ, $1, $3); } 
	| LPAREN expr RPAREN { $$ = $2; }
	| { $$ = NewInt(EMPTY, 0); }
	| COLON date COLON { $$ = NewInt(TICK, $2); printf("TICK %d\n", $2);}
	;

date	: ordinal wday OF month { $$ = nthdayofweek(currentyear, $4, $2, $1); }
	| month mday { $$ = nthday(currentyear, $1, $2); }
	| STRING { $$ = special(currentyear, $1); }
	| date PLUS mday { $$ = addday($1, $3); }
	| date MINUS mday { $$ = addday($1, -$3); }
	| date CASCADE { $$ = cascade($1); }
	;

ordinal : FIRST { $$ = 0; }
	| SECOND { $$ = 1; }
	| THIRD { $$ = 2; }
	| FORTH { $$ = 3; }
	| FIFTH { $$ = 4; }
	| SIXTH { $$ = 5; }
	| SEVENTH { $$ = 6; }
	| EIGHTH { $$ = 7; }
	| NINETH { $$ = 8; }
	| TENTH { $$ = 9; }
	| LAST { $$ = -1; }
	;

month	: JAN { $$ = 0; }
	| FEB { $$ = 1; }
	| MAR { $$ = 2; }
	| APR { $$ = 3; }
	| MAY { $$ = 4; }
	| JUN { $$ = 5; }
	| JUL { $$ = 6; }
	| AUG { $$ = 7; }
	| SEP { $$ = 8; }
	| OCT { $$ = 9; }
	| NOV { $$ = 10; }
	| DEC { $$ = 11; }
	;

wday	: SUN { $$ = 0; }
	| MON { $$ = 1; }
	| TUE { $$ = 2; }
	| WED { $$ = 3; }
	| THU { $$ = 4; }
	| FRI { $$ = 5; }
	| SAT { $$ = 6; }
	;

mday	: INTEGER { $$ = strtol($1, NULL, 10); }
	;
%%
int yyerror(char *str)
{
  fprintf(stderr, "parse error: %s\n", str);
  exit(1);
}
