//extern void yyerror(JS **o, const char* s);
//extern int yyparse(JS**);
//extern int yystart(FILE*);

class collector_t {
 public:
  collector_t(lua_State*);
  int linenum;
  int limit;
  lua_State *L;
  int update(class collect_t*);
  void dump();
};

class collect_t {
 private:
#ifdef WITH_JS
  json *js;
#endif
  collector_t *cl;
 public:
  collect_t();
  collect_t(collector_t*);
  void kv(std::string, std::string);
  void kv(std::string, long);
  void kv(std::string, double);
  void kv(std::string, collect_t&);
  void dump();
};

extern int yylex();
extern int linenum;
extern void yystart(FILE*);
extern double tstod(const char*);
