//extern void yyerror(JS **o, const char* s);
//extern int yyparse(JS**);
//extern int yystart(FILE*);

class collector {
 public:
  collector(lua_State*);
  int linenum;
  int limit;
  lua_State *L;
  int update(class collect*);
  void dump();
};

class collect {
 private:
#ifdef WITH_JS
  json *js;
#endif
  collector *cl;
 public:
  collect();
  collect(collector*);
  void kv(std::string, std::string);
  void kv(std::string, long);
  void kv(std::string, collect&);
  void dump();
};

extern int yylex();
extern int linenum;
extern void yystart(FILE*);
extern double tstod(const char*);
