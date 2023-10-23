namespace fs = std::filesystem;

struct bounds_t {
  double x, y, width, height;
};

class image_t {
protected:
  bounds_t bounds;
  int valid;
public:
  fs::path path;
  image_t();
  image_t(int, int);
  image_t(fs::path);
  virtual int load();
  virtual int save();
  virtual int save(fs::path);
  virtual void fill(std::string);
  virtual image_t *scale(int);
  virtual image_t *scale(double);
  virtual void compose(image_t*, int, int);
  virtual bounds_t frame(double, double, double, double, double, const char*);
  virtual bounds_t text(double, double, double, int, const char*, const char*);
  virtual int get_bounds(bounds_t&);
  int width();
  int height();
  int is_valid();
  std::string description();
};

class gegl_t : public image_t {
private:
  GeglBuffer *gbuffer;
public:
  gegl_t();
  gegl_t(int, int);
  gegl_t(fs::path);
  int load();
  int save();
  int save(fs::path);
  void fill(std::string);
  image_t *scale(int);
  image_t *scale(double);
  void compose(image_t*, int, int);
  bounds_t frame(double, double, double, double, double, const char*);
  bounds_t text(double, double, double, int, const char*, const char*);
  bounds_t get_bounds(std::string);
};

class pixbuf_t : public image_t {
private:
  cairo_surface_t *surface;
public:
  pixbuf_t();
  pixbuf_t(int, int);
  pixbuf_t(fs::path);
  int load();
  int save(fs::path);
  void fill(std::string);
  image_t *scale(double);
  image_t *scale(int);
  void compose(image_t*, int, int);
  bounds_t frame(double, double, double, double, double, const char*);
  bounds_t text(double, double, double, int, const char*, const char*);
};
