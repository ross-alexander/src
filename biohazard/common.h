#define real(x) x

class bio {
public:
  int debug;
  double rotation;
  double inner_radius;
  double inner_offset;
  double outer_radius;
  double outer_offset;
  double center_radius;
  double center_offset;
  double stroke[4];
  double fill[4];
  double line_width;
  double width;
  double height;
  double scale;
  double ring_inner;
  double ring_outer;
  double ring_offset;
  bio(int, char**);
  void cairo(cairo_t*);
  cairo_path_t *ring_path;
  cairo_path_t *horn_path;
};
