extern double deg2rad(double);
extern double rad2deg(double);

class point_t {
public:
  double x, y;
  point_t();
  point_t(const point_t&);
  point_t(double, double);
  point_t& operator+=(const point_t &);
  point_t operator-(const point_t&);
  point_t operator*(const double);
  point_t& operator*=(const double);
  point_t& scale(const double);
  point_t& rotate(double);
  double polar();
  point_t normal();
  void print(const char*s = 0);
};


class line_t {
public:
  point_t p[2];
  line_t();
  line_t(point_t &, point_t &);
  void translate(double);
  void print(const char*);
};

class circle_t {
public:
  double r;
  point_t c;
  circle_t();
  circle_t(double px, double py, double pr);
  static constexpr double eps = 1e-14;
  circle_t(point_t p, double rad);
  circle_t& scale(double s);
  static bool within(double, double, double, double, double, double);
  static int rxy(double, double, double, double, double, double, bool);
  static void isec(point_t&, point_t&, point_t&, double, bool);
  int intersect(const line_t&, point_t&, point_t&);
  int intersect(const circle_t&, point_t&, point_t&);
};
