%{
#include "eval.h"
#include "tokenize.h"
%}

%token MUL DIV ADD SUB INTEGER FLOAT LEFT_PAREN RIGHT_PAREN NEWLINE

%union {
  double d;
  int i;
  expr_t *e;
}

%type <d> FLOAT
%type <i> INTEGER
%type <e> expr

%parse-param { eval_t *eval }

%left ADD SUB
%left MUL DIV
%%
input : %empty
| input line
;

line: expr NEWLINE { expr_print(expr_eval($1)); }

expr : INTEGER { $$ = calloc(sizeof(expr_t), 1); $$->type = INTEGER; $$->i = $1; }
| expr ADD expr { $$ = expr_new_node(ADD, $1, $3); }
| expr SUB expr { $$ = expr_new_node(SUB, $1, $3); }
| expr MUL expr { $$ = expr_new_node(MUL, $1, $3); }
| expr DIV expr { $$ = expr_new_node(DIV, $1, $3); }
| LEFT_PAREN expr RIGHT_PAREN { $$ = $2; }
     /*
     |FLOAT { $$ = $1; }
     */
   ;

