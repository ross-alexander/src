#include <stdio.h>
#include <time.h>
#include <string.h>

#include "common.h"
#include "parse.h"

tree* print(tree* t)
{
  switch(t->op)
    {
    case STRING:
      printf("\"%s\"", (char*)t->s);
      break;
    case TICK:
      {
	char tmp[512];
	time_t tick = t->v;
	strftime(tmp, 512, "%c", localtime(&tick));
	printf("%s", tmp);
	break;
      }
    case INTEGER:
      printf("%d", t->v);
      break;
    default:
      return print(Eval(t));
    }
  return NewInt(INTEGER, 1);
}

tree *omni(tree *t)
{
  if(t->op == TICK)
    {
      time_t tick = t->v;
      char tmp[512];
      strftime(tmp, 512, "%j%t%b %d\t\t# %a %d %b %Y\n", localtime(&tick));
      return NewStr(STRING, strdup(tmp));
    }
  return t;
}
