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
  int v;
} tree;

extern int currentyear;
extern time_t nthday(int, int, int);
extern time_t nthdayofweek(int, int, int, int);
extern time_t cascade(time_t);
extern tree* NewStr(int, char *);
extern tree* NewInt(int, int);
extern tree* NewOp(int, tree*, tree*);
extern tree* Eval(tree*);
extern int True(tree*);

// #define YYPARSE_PARAM restree
