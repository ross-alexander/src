%token FIRST SECOND THIRD FORTH FIFTH SIXTH SEVENTH EIGHTH NINETH TENTH LAST
%token MON TUE WED THU FRI SAT SUN
%token JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC
%token DAY WEEK MONTH YEAR OF SPECIAL CASCADE IDENT
%token INTEGER STRING COMMA PLUS MINUS LPAREN RPAREN NOT AND OR EQ NE TILDE EMPTY FUNC LIST TICK COLON AT

%{
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "common.h"
#include "tokenize.h"

int yyerror(pstate *, char*);
%}

%union {
  int year;
  int mday;
  int wday;
  int month;
  int ordinal;
  time_t date;
  char* text;
}

%type <text> INTEGER STRING
%type <mday> mday
%type <wday> wday
%type <month> month
%type <year> year
%type <ordinal> ordinal time
%type <date> date

%parse-param { pstate* res }

%%
parse	: date { pstate * t = res; t->res = $1; }
	;

date	: ordinal wday OF month { $$ = nthdayofweek(((pstate*)res)->year, $4, $2, $1); }
	| ordinal wday OF month year { $$ = nthdayofweek($5, $4, $2, $1); }
	| ordinal wday OF MONTH month { $$ = nthdayofweek(((pstate*)res)->year, $5, $2, $1); }
	| ordinal wday OF MONTH month AT time { $$ = nthdayofweek(((pstate*)res)->year, $5, $2, $1); }
	| month mday { $$ = nthday(((pstate*)res)->year, $1, $2); }
	| month mday year { $$ = nthday($3, $1, $2); }
	| date PLUS mday { $$ = addday($1, $3); }
	| date MINUS mday { $$ = addday($1, -$3); }
	| STRING { $$ = special(((pstate*)res)->year, $1); }
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
	| INTEGER { $$ = strtol($1, NULL, 10) - 1; }
	;

year	: INTEGER { $$ = strtol($1, 0, 10); }
	| COMMA INTEGER { $$ = strtol($2, 0, 10); }
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

time	: INTEGER COLON INTEGER { $$ = strtol($1, 0,10) * 3600 + strtol($1, 0,10) * 60; }
	| INTEGER COLON INTEGER COLON INTEGER { $$ = strtol($1, 0,10) * 3600 + strtol($1, 0,10) * 60 + strtol($5, 0, 10); }

%%
int yyerror(pstate *res, char *str)
{
  fprintf(stderr, "parse error: %s\n", str);
  exit(1);
}
