typedef struct {
  int cardinal;
  int month;
  int year;
  int day;
} action;

typedef struct tree {
  int op;
  struct tree* left;
  struct tree* right;
  void *s;
  long v;
} tree;

// 2024-12-23: Add yylex

extern int yylex();

extern int currentyear;
extern time_t nthday(int, int, int);
extern time_t nthdayofweek(int, int, int, int);
extern time_t cascade(time_t);
extern tree* NewStr(int, char *);
extern tree* NewInt(int, long);
extern tree* NewOp(int, tree*, tree*);
extern tree* Eval(tree*);
extern int True(tree*);
extern time_t special(int year, char *s);
extern time_t addday(time_t tick, int days);

// #define YYPARSE_PARAM restree
