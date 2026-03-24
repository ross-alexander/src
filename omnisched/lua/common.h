#define YYPARSE_PARAM res

/* ----------------------------------------------------------------------
--
-- Date
--
---------------------------------------------------------------------- */

typedef struct date_t {
  time_t tick;
  struct tm tm;
} date_t;

typedef struct parse_state_t {
  int year;
  time_t res;
} parse_state_t;

extern int parse_date(parse_state_t*, char*);
extern time_t addday(time_t tick, int days);
extern time_t easter(int year);
extern time_t nthday(int year, int month, int day);
extern time_t nthdayofweek(int year, int month, int wday, int n);
extern time_t special(int year, char *s);

