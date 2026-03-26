/* ----------------------------------------------------------------------
   --
   -- Use a virtual base class for eval objects
   --
   ---------------------------------------------------------------------- */

typedef class eval_t* (*evalf)(class roller_t*, class list_t*);

typedef std::map<std::string, evalf> evalf_map;


class roller_t {
public:
  evalf_map fmap;
  int debuglevel;
  roller_t();
};

class eval_t {
 public:
  virtual void dump() = 0;
  virtual class eval_t* eval_f(roller_t*) = 0;
  virtual int eval_l(lua_State*) = 0;
};

class integer_t : public eval_t {
 public:
  int value;
  integer_t(int i);
  void dump();
  eval_t* eval_f(roller_t*);
  int eval_l(lua_State*);
};

class func_t : public eval_t {
  list_t *params;
  std::string function;
public:
  func_t(char *, list_t*);
  void dump();
  eval_t* eval_f(roller_t*);
  int eval_l(lua_State*);
};

class list_t : public eval_t {
public:
  std::vector<eval_t*> list;
  list_t();
  void append(eval_t*);
  void dump();
  eval_t* eval_f(roller_t*);
  int eval_l(lua_State*);
};
