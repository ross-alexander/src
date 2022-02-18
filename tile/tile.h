class tile_format_t {
public:
  std::string path;
  GdkPixbufFormat *format;
  int width;
  int height;
  tile_format_t(std::string);
  ~tile_format_t();
};

class tile_pixbuf_t {
public:
  const char *path;
  GdkPixbuf *pixbuf;
  tile_pixbuf_t();
  tile_pixbuf_t(const char*);
  tile_pixbuf_t(const char*, int, int, gboolean);
  tile_pixbuf_t(GdkPixbuf*);
  ~tile_pixbuf_t();
  int new_from_file(const char*);
  int new_from_file_at_scale(const char*, int, int, gboolean);
  tile_pixbuf_t* scale(uint32_t, uint32_t);
};

class tile_surface_t {
public:
  uint32_t width, height, depth;
  cairo_surface_t *surface;
  tile_surface_t();
  tile_surface_t(uint32_t, uint32_t);
  tile_surface_t(tile_pixbuf_t*);
  virtual ~tile_surface_t();
  void fill(double, double, double);
  void compose(int32_t, int32_t, tile_pixbuf_t*);
};

class tile_surface_xcb_t : public tile_surface_t {
 public:
  xcb_connection_t *conn;
  xcb_window_t root_drawable;
  tile_surface_xcb_t();
  ~tile_surface_xcb_t();
};

extern int tile_surface_t_new_from_xcb(lua_State *L);
extern "C" int luaopen_scanner_t(lua_State *L);
int luaopen_tile_surface_t(lua_State *L);
int luaopen_tile_pixbuf_t(lua_State *L);
int luaopen_tile_format_t(lua_State *L);
