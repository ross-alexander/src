struct dvdrip_t {
  std::string file;
  std::string separator;
  std::string device;
  std::string path;
  std::string lua;
  unsigned int index;
};

struct title_t {
  unsigned int title_nr;
  unsigned int nr_chapters;
  unsigned int duration;
  unsigned int title_set_nr;
  unsigned int vts_ttn;
};

typedef std::vector<title_t*> title_v;

struct playback_time_t {
  unsigned int hour;
  unsigned int minute;
  unsigned int second;
  unsigned int usec;
};


extern lua_State* dvdrip_lua_init(dvdrip_t*);
extern int dvdrip_lua_titles(lua_State *, dvdrip_t*, title_v&);
extern int dvdrip_read_title(dvdrip_t* dvdrip, title_t* title, const char *file);
