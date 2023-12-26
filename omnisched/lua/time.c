#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"

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
