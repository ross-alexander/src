typedef struct eval_t {
  int line_no;
} eval_t;

typedef struct expr_t {
  int type;
  union {
    double d;
    int i;
    struct {
      struct expr_t *left, *right;
    } tree;
  };
} expr_t;

extern void yyerror(eval_t*, const char*);
extern expr_t* expr_eval(expr_t*);
extern void expr_print(expr_t*);
extern expr_t* expr_new_node(int i, expr_t*, expr_t*);
