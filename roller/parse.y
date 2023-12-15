%{
#include <array>
#include <vector>
#include <map>

#include <stdio.h>
#include "common.h"
  
  extern int yylex();
  extern int yyerror(eval_t**, const char*);
%}

// Only support integers and constructed eval objects

%union {
  int i;
  eval_t *e;
  list_t *list;
  char* ident;
}

%token INTEGER PLUS MINUS DIE TIMES LPAREN RPAREN LSQUARE RSQUARE IDENT COMMA
%type <i> INTEGER
%type <e> expr
%type <list> exprlist
%type <ident> IDENT

 // Operator orders and priority

%left PLUS MINUS
%left TIMES
%left DIE

 // Result returned by updating a pointer to eval_t*

%parse-param { eval_t **eval }


%%
all	: expr { *eval = $1; }
	;

exprlist : exprlist COMMA expr { $$->append($3); printf("exprlist + expr\n"); }
| expr { $$ = new list_t(); $$->append($1); printf("expr in exprlist\n"); }
| { $$ = new list_t(); }
        ;

expr    : INTEGER { $$ = new integer_t($1); }
        | DIE INTEGER { $$ = new die_t(1, $2); }
        | INTEGER DIE INTEGER { $$ = new die_t($1, $3); }
        | expr PLUS expr { $$ = new plus_t($1, $3); }
        | expr MINUS expr { $$ = new minus_t($1, $3); }
        | expr TIMES expr { $$ = new times_t($1, $3); }
        | LPAREN expr RPAREN { $$ = new group_t($2); }
        | IDENT LPAREN exprlist RPAREN { $$ = new func_t($1, $3); }
        ;
