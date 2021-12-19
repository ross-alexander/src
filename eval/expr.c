#include <stdlib.h>
#include <stdio.h>

#include "eval.h"
#include "parse.tab.h"

/* ----------------------------------------------------------------------
--
-- eval_expr
--
---------------------------------------------------------------------- */

expr_t* expr_eval(expr_t *e)
{
  expr_t *left, *right;
  switch(e->type)
    {
    case INTEGER:
      printf("%d ", e->i);
      break;
    case ADD:
      left = expr_eval(e->tree.left);
      right = expr_eval(e->tree.right);
      printf("+ ");
      if (left->type == INTEGER && right->type == INTEGER)
	{
	  e->type = INTEGER;
	  e->i = left->i + right->i;
	}
      break;
    case SUB:
      left = expr_eval(e->tree.left);
      right = expr_eval(e->tree.right);
      printf("- ");
      if (left->type == INTEGER && right->type == INTEGER)
	{
	  e->type = INTEGER;
	  e->i = left->i - right->i;
	}
      break;
    case MUL:
      left = expr_eval(e->tree.left);
      right = expr_eval(e->tree.right);
      printf("* ");
      if (left->type == INTEGER && right->type == INTEGER)
	{
	  e->type = INTEGER;
	  e->i = left->i * right->i;
	}
      break;
    case DIV:
      left = expr_eval(e->tree.left);
      right = expr_eval(e->tree.right);
      printf("/ ");
      if (left->type == INTEGER && right->type == INTEGER)
	{
	  e->type = INTEGER;
	  e->i = left->i / right->i;
	}
      break;
    }
  return e;
}

void expr_print(expr_t *e)
{
  switch(e->type)
    {
    case INTEGER:
      printf("%d\n", e->i);
      break;
    }
}

expr_t* expr_new_node(int t, expr_t* l, expr_t *r)
{
  expr_t* d = (expr_t*)calloc(sizeof(expr_t), 1);
  d->type = t;
  d->tree.left = l;
  d->tree.right = r;
  return d;
}
