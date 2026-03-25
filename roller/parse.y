%{
#include <array>
#include <vector>
#include <map>
#include <string>

#include <stdio.h>

#include <lua.hpp>
  
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

%token INTEGER PLUS MINUS DIE TIMES IDENT COMMA BANG
%token LPAREN RPAREN LSQUARE RSQUARE LBRACE RBRACE
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

exprlist : exprlist COMMA expr
	{
	  $$->append($3);
	}
	| expr
	{
	  $$ = new list_t();
	  $$->append($1);
	}
	|
	{
	  $$ = new list_t();
	}
        ;

expr    : INTEGER
	{
	  $$ = new integer_t($1);
	}
        | DIE INTEGER
	{
	  list_t *params = new list_t();
	  params->append(new integer_t(1));
	  params->append(new integer_t($2));
	  params->append(new integer_t(0));
	  $$ = new func_t((char*)"die", params);
	}
        | INTEGER DIE INTEGER
	{
	  list_t *params = new list_t();
	  params->append(new integer_t($1));
	  params->append(new integer_t($3));
	  params->append(new integer_t(0));
	  $$ = new func_t((char*)"die", params);
	}
        | DIE INTEGER BANG
	{
	  list_t *params = new list_t();
	  params->append(new integer_t(1));
	  params->append(new integer_t($2));
	  params->append(new integer_t(1));
	  $$ = new func_t((char*)"die", params);
	}
        | INTEGER DIE INTEGER BANG
	{
	  list_t *params = new list_t();
	  params->append(new integer_t($1));
	  params->append(new integer_t($3));
	  params->append(new integer_t(1));
	  $$ = new func_t((char*)"die", params);
	}
        | expr PLUS expr
	{
	  list_t *params = new list_t();
	  params->append($1);
	  params->append($3);
	  $$ = new func_t((char*)"__plus", params);
	}
        | expr MINUS expr
	{
	  list_t *params = new list_t();
	  params->append($1);
	  params->append($3);
	  $$ = new func_t((char*)"__minus", params);
 	}
        | expr TIMES expr
	{
	  list_t *params = new list_t();
	  params->append($1);
	  params->append($3);
	  $$ = new func_t((char*)"__times", params);
	}
	| LSQUARE expr RSQUARE
	{
	  list_t *params = new list_t();
	  params->append($2);
	  $$ = new func_t((char*)"__floor", params);
	}
	| LPAREN expr RPAREN
	{
	  $$ = $2;
	}
	| LBRACE exprlist RBRACE
	{
	  $$ = $2;
	}
        | IDENT LPAREN exprlist RPAREN
	{
	  $$ = new func_t($1, $3);
	}
        ;
