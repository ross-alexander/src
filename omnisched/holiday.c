#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <dlfcn.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif


#include "common.h"
#include "parse.h"

/* ----------------------------------------------------------------------
--
-- addday
--
---------------------------------------------------------------------- */

time_t addday(time_t tick, int days)
{
  tick += days * 60 * 60 * 24;
  return tick;
}

/* ----------------------------------------------------------------------
--
-- easter
--
---------------------------------------------------------------------- */

time_t easter(int year)
{
  int G = year % 19;
  int C = year / 100;
  int H = (C - C/4 - (8*C+13)/25 + 19*G + 15) % 30;
  int I = H - (H/28)*(1-(H/28)*(29/(H+1))*((21-G)/11));
  int J = (year + year/4 + I + 2 - C + C/4) % 7;
  int L = I - J;
  int EasterMonth = 3 + (L + 40) / 44;
  int EasterDay = L + 28 - 31 * (EasterMonth / 4);
  time_t tick;
  struct tm *stm = calloc(sizeof(struct tm), 1);
  stm->tm_year = year - 1900;
  stm->tm_mon = EasterMonth - 1;
  stm->tm_mday = EasterDay;
  stm->tm_hour = 12;
  tick = mktime(stm);
  free(stm);
  return tick;
}

/* ----------------------------------------------------------------------
--
-- nthday
--
-- Returns the nth day of a month
--
---------------------------------------------------------------------- */
time_t nthday(int year, int month, int day)
{
  time_t tick;
  struct tm *stm = calloc(sizeof(struct tm), 1);
  stm->tm_year = year - 1900;
  stm->tm_mon = month;
  stm->tm_mday = day;
  stm->tm_hour = 12;
  tick = mktime(stm);
  free(stm);
  return tick;
}

/* ----------------------------------------------------------------------
--
-- nthdayofweek
--
-- Returns the nths day of the month (ie the first friday on the month
--
---------------------------------------------------------------------- */

time_t nthdayofweek(int year, int month, int wday, int n)
{
  time_t tick;
  struct tm *stm = calloc(sizeof(struct tm), 1);

/* --------------------
   Get first of the month (unless getting last day)
   -------------------- */

  stm->tm_year = year - 1900;
  stm->tm_mday = 1;
  stm->tm_mon = n >= 0 ? month : month + 1;
  stm->tm_hour = 12;
  stm->tm_min = 0;
  stm->tm_sec = 0;

  tick = mktime(stm);
  localtime_r(&tick, stm);
  if (n >= 0)
    tick = addday(tick, ((7 - (stm->tm_wday - wday)) % 7) + (n * 7));
  else
    tick = addday(tick, ((7 - (stm->tm_wday - wday)) % 7) - 7);
  free(stm);
  return tick;
}

/* ----------------------------------------------------------------------
--
-- cascade
--
---------------------------------------------------------------------- */
time_t cascade(time_t t)
{
  struct tm *tm = localtime(&t);
  if (tm->tm_wday == 0)
    t += 86400;
  else if (tm->tm_wday == 6)
    t += 86400*2;
  return t;
}

/* ----------------------------------------------------------------------
--
-- special
--
---------------------------------------------------------------------- */
time_t special(int year, char *s)
{
  if (!strcmp("easter", s))
    {
      return easter(year);
    }
  return 0;
}

/* ----------------------------------------------------------------------
--
-- abc
--
---------------------------------------------------------------------- */
tree* abc(tree* t)
{
  printf("ABCBCABSD\n");
  return NewInt(INTEGER, 1);
}


/* ----------------------------------------------------------------------
--
-- NewValue
--
---------------------------------------------------------------------- */
tree *NewStr(int v, char *s)
{
  tree *t = calloc(sizeof(tree), 1);
  t->op = v;
  t->s = s;
  return t;
}

tree *NewInt(int v, int s)
{
  tree *t = calloc(sizeof(tree), 1);
  t->op = v;
  t->v = s;
  return t;
}

tree *NewOp(int o, tree *l, tree *r)
{
  tree *t = calloc(sizeof(tree), 1);
  t->op = o;
  t->left = l;
  t->right = r;
  return t;
}


tree *Eval(tree *t)
{
  int a;
  switch(t->op)
    {
    case INTEGER:
    case IDENT:
    case STRING:
    case TICK:
    case EMPTY:
      return t;
      break;
    case NOT:
      a = True(t->left);
      printf("Not(%d %d)\n", a, !a);
      return NewInt(INTEGER, !a);
      break;
    case FUNC:
      {
	char *s = t->left->s;
	assert(s != NULL);
	printf("Func(%s)\n", s);
	void *v = dlsym(RTLD_DEFAULT, s);
	tree* (*f)(tree*) = v;
	if (f)
	  return (*f)(t->right);
	else
	  return NewInt(INTEGER, 0);
	break;
      }
    }
  return NULL;
}

int True(tree *t)
{
  switch (t->op)
    {
    case EMPTY:
      printf("True(EMPTY)\n");
      return 0;
      break;
    case INTEGER:
    case TICK:
      printf("True(%d)\n", t->v);
      return t->v;
      break;
    case STRING:
      if (t->s)
	return strlen(t->s);
      else
	return 0;
      break;
    default:
      return True(Eval(t));
      break;
    }
}

/* ----------------------------------------------------------------------
--
-- 
---------------------------------------------------------------------- */
void Process(tree *t)
{
  if (t->op == LIST)
    {
      Process(t->left);
      if (t->right) Process(t->right);
    }
  else if (t->op == TILDE)
    {
      printf("TILDE(%p %p)\n", t->left, t->right);
      if (True(t->right))
	Process(t->left);
    }
  else if (t->op == TICK)
    {
      time_t tick = t->v;
      char tmp[512];
      strftime(tmp, 512, "%j%t%b %d\t\t# %a %d %b %Y\n", localtime(&tick));
      printf("%s", tmp);
    }
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

extern int yydebug;
int currentyear;

int main(int argc, char *argv[])
{
  int ch;

  struct tm tm;
  time_t tick = time(NULL);
  localtime_r(&tick, &tm);
  currentyear = 1900 + tm.tm_year;

  while((ch = getopt(argc, argv, "dy:")) != EOF)
    switch (ch)
      {
      case 'd':
	yydebug=1;
	break;
      case 'y':
	currentyear = strtol(optarg, NULL, 10);
	break;
      }
  tree *res;
  if (yyparse(&res) == 0)
    {
      printf("Res = %p\n", res);
      Process(res);
    }
  else
    {
      fprintf(stderr, "Parse failed.\n");
    }
  exit(0);
}
