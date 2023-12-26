#define YYPARSE_PARAM res

/* ----------------------------------------------------------------------
--
-- Date
--
---------------------------------------------------------------------- */

typedef struct Date {
  time_t tick;
  struct tm tm;
} Date;

typedef struct ParseState {
  int year;
  time_t res;
} pstate;

int parse_date(pstate*, char*);
time_t addday(time_t tick, int days);
time_t easter(int year);
time_t nthday(int year, int month, int day);
time_t nthdayofweek(int year, int month, int wday, int n);
time_t special(int year, char *s);

