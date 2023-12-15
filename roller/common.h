/* ----------------------------------------------------------------------
   --
   -- Use a virtual base class for eval objects
   --
   ---------------------------------------------------------------------- */

class eval_t {
 public:
  virtual void dump() = 0;
  virtual int eval() = 0;
};

class integer_t : public eval_t {
  int value;
 public:
  integer_t(int i);
  void dump();
  int eval();
};

class plus_t : public eval_t {
  eval_t *left, *right;
 public:
  plus_t(eval_t*, eval_t*);
  void dump();
  int eval();
};


class minus_t : public eval_t {
  eval_t *left, *right;
 public:
  minus_t(eval_t*, eval_t*);
  void dump();
  int eval();
};

class times_t : public eval_t {
  eval_t *left, *right;
 public:
  times_t(eval_t*, eval_t*);
  void dump();
  int eval();
};

class die_t : public eval_t {
  int count, die;
 public:
  die_t(int, int);
  void dump();
  int eval();
};

class group_t : public eval_t {
  eval_t *group;
public:
  group_t(eval_t*);
  void dump();
  int eval();
};

class func_t : public eval_t {
  eval_t *group;
  char *function;
public:
  func_t(char *, eval_t*);
  void dump();
  int eval();
};

class list_t : public eval_t {
  std::vector<eval_t*> list;
public:
  list_t();
  void append(eval_t*);
  void dump();
  int eval();
};
