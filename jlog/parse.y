%{
#include <stdio.h>

#include <string>
#include <iostream>
#include <map>

#include <jsoncons/json.hpp>
#include <lua.hpp>

using jsoncons::json;

#include "common.h"

extern void yyerror(collector*, const char*);

#define YYDEBUG 1

%}

%token IDENT INTEGER TS COLON PERIOD HYPHEN EOL LSQUARE RSQUARE ASSIGN STRING IPV4 TIME

%union {
  std::string *s;
  std::map<std::string, std::string> *m;
  jsoncons::json *j;
  collect *c;
}

%type <s> IDENT INTEGER TS STRING IPV4 TIME
%type <j> line file
%type <c> sink res

%parse-param { collector *cl }

%%
res : file { /* *res = $1; */ }
;

file : line { }
| file line { if (cl->limit && cl->linenum > cl->limit) return 0; }
;

/*line : IDENT INTEGER TIME IPV4 INTEGER TS IDENT IDENT HYPHEN IDENT LSQUARE IDENT sink RSQUARE EOL */

line : IDENT TS TS IDENT IDENT IDENT LSQUARE IDENT sink RSQUARE EOL
{
  collect *c = new collect(cl);
  c->kv("linenum", cl->linenum++);

  c->kv("syslog-ts", tstod($2->c_str()));
  c->kv("timestamp", tstod($3->c_str()));
  c->kv("action", *$6);
  c->kv("jlog", *$9);
  int ret = cl->update(c);

  delete $1; delete $2; delete $3; delete $4; delete $5; delete $6;  // delete $7; delete $8; delete $10; delete $12;
  delete(c);
  if (ret < 0) return(ret);
}
;

sink : IDENT ASSIGN STRING { $$ = new collect(cl); $$->kv(*$1, *$3); delete $1; delete $3;}
| sink IDENT ASSIGN STRING { $$ = $1;  $1->kv(*$2, *$4); delete $2; delete $4; }
;

%%

/* ----------------------------------------------------------------------
--
-- yyerror
--
---------------------------------------------------------------------- */

void yyerror(collector *cl, const char *s)
{
  //  printf("%s\n", (*js)->stringify());
  fprintf(stderr, "%s on line %d\n", s, 0);
  exit(1);
}

