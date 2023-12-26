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

extern int parse_date(pstate*, char*);
extern time_t addday(time_t tick, int days);
extern time_t easter(int year);
extern time_t nthday(int year, int month, int day);
extern time_t nthdayofweek(int year, int month, int wday, int n);
extern time_t special(int year, char *s);

