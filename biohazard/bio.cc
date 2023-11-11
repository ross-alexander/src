#include <iostream>
#include <string>
#include <complex>

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <cairo-svg.h>
#include <json-c/json.h>
#include "common.h"

/* ----------------------------------------------------------------------
--
-- bio.cc
--
-- 2019-06-02
--   Update to match json-c 0.13.1
--
---------------------------------------------------------------------- */

double deg2rad(double d)
{
  return d * M_PI / 180;
}

double rad2deg(double d)
{
  return d * 180 / M_PI;
}

double square(double d)
{
  return d*d;
}

int circle_circle_intersection(double x0, double y0, double r0,
                               double x1, double y1, double r1,
                               double *xi, double *yi,
                               double *xi_prime, double *yi_prime)
{
  double a, dx, dy, d, h, rx, ry;
  double x2, y2;

  /* dx and dy are the vertical and horizontal distances between
   * the circle centers.
   */
  dx = x1 - x0;
  dy = y1 - y0;

  /* Determine the straight-line distance between the centers. */
  
  //d = sqrt((dy*dy) + (dx*dx));
  
  d = hypot(dx, dy); // Suggested by Keith Briggs

  assert(!(d > (r0 + r1)));

  /* Check for solvability. */
  if (d > (r0 + r1))
  {
    /* no solution. circles do not intersect. */
    return 0;
  }

  if (d - fabs(r0 - r1) < 0.0)
  {
    /* no solution. one circle is contained in the other */
    return 0;
  }

  /* 'point 2' is the point where the line through the circle
   * intersection points crosses the line between the circle
   * centers.  
   */

  /* Determine the distance from point 0 to point 2. */
  a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0 * d) ;

  /* Determine the coordinates of point 2. */
  x2 = x0 + (dx * a/d);
  y2 = y0 + (dy * a/d);

  /* Determine the distance from point 2 to either of the
   * intersection points.
   */
  h = sqrt((r0*r0) - (a*a));

  /* Now determine the offsets of the intersection points from
   * point 2.
   */
  rx = -dy * (h/d);
  ry = dx * (h/d);

  /* Determine the absolute intersection points. */
  *xi = x2 + rx;
  *xi_prime = x2 - rx;
  *yi = y2 + ry;
  *yi_prime = y2 - ry;

  return 1;
}

/* https://paulbourke.net/geometry/circlesphere/source.cpp */

int circle_line_intersection (double x1, double y1,
			      double x2, double y2,
			      double x3, double y3, double r,
			      double *rx1, double *ry1, double *rx2, double *ry2)
{

  // x1,y1,z1  P1 coordinates (point of line)
  // x2,y2,z2  P2 coordinates (point of line)
  // x3,y3,z3, r  P3 coordinates and radius (sphere)
  // x,y,z   intersection coordinates
  //
  // This function returns a pointer array which first index indicates
  // the number of intersection point, followed by coordinate pairs.
  
  double x, y, z;
  double a, b, c, mu, i;

  a =  square(x2 - x1) + square(y2 - y1);
  b =  2 * ((x2 - x1)*(x1 - x3) + (y2 - y1)*(y1 - y3));
  c =  square(x3) + square(y3) + square(x1) + square(y1) - 2* ( x3*x1 + y3*y1) - square(r) ;
  i =   b * b - 4 * a * c ;
  
  if (i < 0.0)
    {
      // no intersection
      return 0;
    }
  
  if (i == 0.0)
    {
      mu = -b/(2*a) ;
      *rx1 = x1 + mu*(x2-x1);
      *ry1 = y1 + mu*(y2-y1);
      return 1;
    }
  if (i > 0.0)
    {
      // first intersection
      mu = (-b + sqrt( square(b) - 4*a*c )) / (2*a);
      *rx1 = x1 + mu*(x2-x1);
      *ry1 = y1 + mu*(y2-y1);
      // second intersection
      mu = (-b - sqrt(square(b) - 4*a*c )) / (2*a);

      *rx2 = x1 + mu*(x2-x1);
      *ry2 = y1 + mu*(y2-y1);
      return 2;
    }
  return 0;
}

/* ----------------------------------------------------------------------
--
-- Point
--
---------------------------------------------------------------------- */

class Point {
public:
  double x, y;
  Point() { x = y = 0.0; }
  Point(const Point& p) { x = p.x; y = p.y; }
  Point(double px, double py)
  {
    x = px;
    y = py;
  }
  Point operator-(const Point& p)
  {
    Point r(x-p.x, y-p.y);
    return r;
  }
  Point operator*(const double l)
  {
    return Point(x*l, y*l);
  }

  Point& operator*=(const double l)
  {
    x *= l;
    y *= l;
    return *this;
  }

  Point& scale(const double l)
  {
    x *= l;
    y *= l;
    return *this;
  }

  Point& operator+=(const Point &p)
  {
    x += p.x;
    y += p.y;
    return *this;
  }
  Point& rotate(double theta)
  {
    //    theta = deg2rad(theta);
    double xr = cos(theta) * x - sin(theta) * y;
    double yr = sin(theta) * x + cos(theta) * y;
    x = xr;
    y = yr;
    return *this;
  }
  double polar()
  {
    double theta = atan(double(y) / double(x));
    double phi;

    if (double(x) < 0.0 && double(y) > 0.0)
      phi = theta + M_PI;
    else if (double(x) < 0.0 && double(y) < 0.0)
      phi = theta + M_PI;
    else if (double(x) > 0.0 && double(y) < 0.0)
      phi = theta + 2*M_PI;
    else
      phi = theta;

    //    printf("polar(%f, %f = %f)\n", double(x), double(y), rad2deg(phi));
    return phi;
  }
  Point normal()
  {
    double l = sqrt(x * x + y * y);
    return Point(x/l, y/l);
  }
};

/* ----------------------------------------------------------------------
--
-- Line
--
---------------------------------------------------------------------- */

class Line {
public:
  Point p[2];
  Line() {};
  Line(Point p1, Point p2) { p[0] = p1; p[1] = p2; }
  void draw(cairo_t *cr)
  {
    cairo_new_path(cr);
    cairo_move_to(cr, double(p[0].x), double(p[0].y));
    cairo_line_to(cr, double(p[1].x), double(p[1].y));
    cairo_stroke(cr);
  }
  void translate(double s)
  {
    Point x = (p[1] - p[0]).normal();
    x.rotate(-90);
    p[0] += x * s;
    p[1] += x * s;
  }
  void dump(const char* s = "")
  {
    printf("Line %s (%f, %f) -> (%f, %f)\n", s, p[0].x, p[0].y, p[1].x, p[1].y);
  }
};

/* ----------------------------------------------------------------------
--
-- Circle
--
---------------------------------------------------------------------- */

class Circle {
public:
  double r;
  Point c;
  Circle() { r = 0.0; }
  Circle(double px, double py, double pr)
  {
    c.x = px;
    c.y = py;
    r = pr;
  };
  static constexpr double eps = 1e-14;

  Circle(Point p, double rad)
  {
    c = p;
    r = rad;
  }

  Circle& scale(double s)
  {
    c.scale(s);
    r *= s;
    return *this;
  }

  void draw(cairo_t *cr) {
    cairo_new_path(cr);
    cairo_move_to(cr, c.x - 1.0, c.y);
    cairo_line_to(cr, c.x + 1.0, c.y);
    cairo_move_to(cr, c.x, c.y - 1.0);
    cairo_line_to(cr, c.x, c. y + 1.0);
    cairo_new_sub_path(cr);
    cairo_arc(cr, c.x, c.y, r, 0, deg2rad(360));
    cairo_stroke(cr);
  }

bool within(double x1, double y1, double x2, double y2, double x, double y) {
    double d1 = sqrt(square(x2 - x1) + square(y2 - y1));    // distance between end-points
    double d2 = sqrt(square(x - x1) + square(y - y1));      // distance from point to one end
    double d3 = sqrt(square(x2 - x) + square(y2 - y));      // distance from point to other end
    double delta = d1 - d2 - d3;
    return fabs(delta) < eps;   // true if delta is less than a small tolerance
}
  
  int rxy(double x1, double y1, double x2, double y2, double x, double y, bool segment) {
    if (!segment || within(x1, y1, x2, y2, x, y)) {
      std::cout << "** " << x << " " << y << "\n";
        return 1;
    } else {
        return 0;
    }
}

double fx(double A, double B, double C, double x) {
    return -(A * x + C) / B;
}

double fy(double A, double B, double C, double y) {
    return -(B * y + C) / A;
}

  // Prints the intersection points (if any) of a circle, center 'cp' with radius 'r',
  // and either an infinite line containing the points 'p1' and 'p2'
  // or a segment drawn between those points.
  void isec(Point p1, Point p2, Point cp, double r, int segment)
  {
    double x0 = cp.x, y0 = cp.y;
    double x1 = p1.x, y1 = p1.y;
    double x2 = p2.x, y2 = p2.y;
    double A = y2 - y1;
    double B = x1 - x2;
    double C = x2 * y1 - x1 * y2;
    double a = square(A) + square(B);
    double b, c, d;
    bool bnz = true;
    int cnt = 0;

    
    if (fabs(B) >= eps) {
        // if B isn't zero or close to it
        b = 2 * (A * C + A * B * y0 - square(B) * x0);
        c = square(C) + 2 * B * C * y0 - square(B) * (square(r) - square(x0) - square(y0));
    } else {
        b = 2 * (B * C + A * B * x0 - square(A) * y0);
        c = square(C) + 2 * A * C * x0 - square(A) * (square(r) - square(x0) - square(y0));
        bnz = false;
    }
    d = square(b) - 4 * a * c; // discriminant
    if (d < 0) {
        // line & circle don't intersect
        printf("[]\n");
        return;
    }

    if (d == 0) {
        // line is tangent to circle, so just one intersect at most
        if (bnz) {
            double x = -b / (2 * a);
            double y = fx(A, B, C, x);
            cnt = rxy(x1, y1, x2, y2, x, y, segment);
        } else {
            double y = -b / (2 * a);
            double x = fy(A, B, C, y);
            cnt = rxy(x1, y1, x2, y2, x, y, segment);
        }
    } else {
        // two intersects at most
        d = sqrt(d);
        if (bnz) {
            double x = (-b + d) / (2 * a);
            double y = fx(A, B, C, x);
            cnt = rxy(x1, y1, x2, y2, x, y, segment);

            x = (-b - d) / (2 * a);
            y = fx(A, B, C, x);
            cnt += rxy(x1, y1, x2, y2, x, y, segment);
        } else {
            double y = (-b + d) / (2 * a);
            double x = fy(A, B, C, y);
            cnt = rxy(x1, y1, x2, y2, x, y, segment);

            y = (-b - d) / (2 * a);
            x = fy(A, B, C, y);
            cnt += rxy(x1, y1, x2, y2, x, y, segment);
        }
    }

    if (cnt <= 0) {
        printf("[]");
    }
  }

  
  int intersect(const Line& l, Point &p1, Point &p2)
  {
    double x1, y1, x2, y2;
    int res;
    assert(res = circle_line_intersection(l.p[0].x, l.p[0].y, l.p[1].x, l.p[1].y, c.x, c.y, r,
					  &x1, &y1, &x2, &y2) > 0);
    p1.x = x1; p1.y = y1;
    p2.x = x2; p2.y = y2;
    return res;
  }

  int intersect(const Circle &cir, Point &p1, Point &p2)
  {
    int res;

    assert(res = circle_circle_intersection(double(c.x), double(c.y), r,
					    double(cir.c.x), double(cir.c.y), cir.r,
					    &p1.x, &p1.y, &p2.x, &p2.y) > 0);
    return res;
  }
};

/* ----------------------------------------------------------------------
--
-- bio_cairo_2
--
---------------------------------------------------------------------- */


void bio_cairo_2(cairo_t *cr, bio_t &v)
{
  int w = v.width;
  int h = v.height;

  cairo_save(cr);

  // Move center to middle of page
  cairo_translate(cr, w/2, h/2);

  // Y-Invert
  cairo_scale(cr, 1.0, -1.0);

  double scale = v.scale;

  /* ----------------------------------------
     New construction
     ---------------------------------------- */
  
  if (v.horn_path == nullptr)
    {
      Point left_outer_start_p(v.outer_offset + v.outer_radius, 0);
      left_outer_start_p.rotate(150);
      left_outer_start_p.scale(scale);
      Line left_outer_start_l(Point(0,0), left_outer_start_p);
      
      Point right_outer_start_p(v.outer_offset + v.outer_radius, 0);
      right_outer_start_p.rotate(30);
      right_outer_start_p.scale(scale);
      Line right_30deg(Point(0,0), right_outer_start_p);
      
      // left_outer_start_l.draw(cr);
      // right_30deg.draw(cr);
      
      Circle outer_circle(0, v.outer_offset, v.outer_radius);
      outer_circle.scale(scale);

      // outer_circle.draw(cr);
      
      Circle inner_circle(0, v.inner_offset, v.inner_radius);
      inner_circle.scale(scale);

      // inner_circle.draw(cr);
      
      Circle center_circle(0, 0, v.center_radius);
      center_circle.scale(scale);

      // center_circle.draw(cr);
      
      Line vertical_left(Point(0, 0), Point(0, scale * v.inner_offset));
      vertical_left.translate(-scale * v.center_offset);

      // vertical_left.draw(cr);
      
      Line vertical_right(Point(0, 0), Point(0, scale * v.inner_offset));
      vertical_right.translate(+scale * v.center_offset);

      // vertical_right.draw(cr);
      
      Point vertical_right_inner[2];
      inner_circle.intersect(vertical_right, vertical_right_inner[0], vertical_right_inner[1]);
      
      Point inner_outer[2];
      inner_circle.intersect(outer_circle, inner_outer[0], inner_outer[1]);
      
      Point outer_right_30deg[2];
      
      outer_circle.intersect(right_30deg, outer_right_30deg[0], outer_right_30deg[1]);
     
      Point center_right[2];
      center_circle.intersect(vertical_right, center_right[0], center_right[1]);
      
      double ang_inner_r_start = (vertical_right_inner[1] - inner_circle.c).polar();
      double ang_inner_r_end = (inner_outer[0] - inner_circle.c).polar();
      double ang_outer_r_start = (inner_outer[0] - outer_circle.c).polar();
      double ang_outer_r_end = (outer_right_30deg[0] - outer_circle.c).polar();
      double ang_center_r_end = center_right[0].polar();

  /* --------------------
     Do clockwise path
     -------------------- */

      cairo_save(cr);
      cairo_new_path(cr);

      /* Center arc - the lines between the arcs are implicit in the path */

      cairo_arc(cr, 0.0, 0.0, v.center_radius * scale, deg2rad(30), ang_center_r_end);
      
      /* inside right hand side arc of horn */

      cairo_arc(cr, inner_circle.c.x, inner_circle.c.y, inner_circle.r, ang_inner_r_start, ang_inner_r_end);
      
      /* outside right hand side arc of horn */


      cairo_arc_negative(cr, outer_circle.c.x, outer_circle.c.y, outer_circle.r, ang_outer_r_start, ang_outer_r_end);

      //      cairo_path_t *horn_cw_path = cairo_copy_path(cr);
      
  /* --------------------
     Do anticlockwise path
     -------------------- */

      cairo_scale(cr, 1, -1);
      cairo_rotate(cr, deg2rad(-60));
      
      /* outside right hand side arc of horn */
      cairo_arc(cr, outer_circle.c.x, outer_circle.c.y, outer_circle.r, ang_outer_r_end, ang_outer_r_start);
      /* inside right hand side arc of horn */
      cairo_arc_negative(cr, inner_circle.c.x, inner_circle.c.y, inner_circle.r, ang_inner_r_end, ang_inner_r_start);
      /* Center arc - the lines between the arcs are implicit in the path */
      cairo_arc_negative(cr, 0.0, 0.0, v.center_radius * scale, ang_center_r_end, deg2rad(30));

      cairo_close_path(cr);
      v.horn_path = cairo_copy_path(cr);
      cairo_restore(cr);
    }

  /* --------------------
     ring
     -------------------- */

  if (v.ring_path == nullptr)
    {
      Circle ring_offset(0, v.inner_offset, (v.inner_radius - v.ring_offset));
      ring_offset.scale(scale);
      
      Circle ring_outer(0, 0, v.ring_outer);
      ring_outer.scale(scale);
      Circle ring_inner(0, 0, v.ring_inner);
      ring_inner.scale(scale);
      
      Point ring_outer_intsec[2];
      Point ring_inner_intsec[2];
      
      ring_inner.intersect(ring_offset, ring_inner_intsec[0], ring_inner_intsec[1]);
      ring_outer.intersect(ring_offset, ring_outer_intsec[0], ring_outer_intsec[1]);

      double ang_ring_outer_start = ring_outer_intsec[0].polar();
      double ang_ring_outer_end = ring_outer_intsec[1].polar();
      double ang_ring_inner_start = ring_inner_intsec[1].polar();
      double ang_ring_inner_end = ring_inner_intsec[0].polar();
      
      double ang_ring_offset_r_start = (ring_outer_intsec[1] - ring_offset.c).polar();
      double ang_ring_offset_r_end = (ring_inner_intsec[1] - ring_offset.c).polar();
      
      double ang_ring_offset_l_start = (ring_inner_intsec[0] - ring_offset.c).polar();
      double ang_ring_offset_l_end = (ring_outer_intsec[0] - ring_offset.c).polar();

      cairo_new_path(cr);
      cairo_arc_negative(cr, 0, 0, v.ring_outer * scale, ang_ring_outer_start, ang_ring_outer_end);
      cairo_arc_negative(cr, ring_offset.c.x, ring_offset.c.y, ring_offset.r, ang_ring_offset_r_start, ang_ring_offset_r_end);
      cairo_arc(cr, 0, 0, v.ring_inner * scale, ang_ring_inner_start, ang_ring_inner_end);
      cairo_arc_negative(cr, ring_offset.c.x, ring_offset.c.y, ring_offset.r, ang_ring_offset_l_start, ang_ring_offset_l_end);
      cairo_close_path(cr);
      v.ring_path = cairo_copy_path(cr);
    }

  /* --------------------
     The actual drawing
     -------------------- */

  cairo_set_line_width(cr, v.line_width);

  assert(v.ring_path);
  assert(v.horn_path);

  for (int i = 0; i < 3; i++)
    {
      cairo_save(cr);
      cairo_rotate(cr, deg2rad(120 * i + v.rotation));
      cairo_new_path(cr);

      cairo_append_path(cr, v.horn_path);
      cairo_append_path(cr, v.ring_path);

      //      cairo_set_source_rgba(cr, v.fill[0], v.fill[1], v.fill[2], v.fill[3]);

      cairo_pattern_t *p = cairo_pattern_create_linear(0, 0, v.width/2, v.height/2);
      cairo_pattern_add_color_stop_rgba(p, 0, 0, 1, 0, 1);
      cairo_pattern_add_color_stop_rgba(p, 1, 0, 0, 1, 1);
      cairo_set_source(cr, p);

      cairo_fill_preserve(cr);
      cairo_set_source_rgba(cr, v.stroke[0], v.stroke[1], v.stroke[2], v.stroke[3]);
      cairo_stroke(cr);
      cairo_restore(cr);
    }

  cairo_restore(cr);

  return;
}

/* ----------------------------------------------------------------------
   --
   -- bio_cairo
   --
   ---------------------------------------------------------------------- */

void bio_cairo(cairo_t *cr, bio_t &v)
{
  int w = v.width;
  int h = v.height;

  cairo_save(cr);

  // Move center to middle of page
  cairo_translate(cr, w/2, h/2);

  // Y-Invert
  cairo_scale(cr, 1.0, -1.0);

  double scale = v.scale;

  /* The trefoil is made from three rotationally symetric parts.
     Each part has 7 points.
  */

  Circle inner[3];
  Circle outer[3];

  Point in_out[3][2];
  Point out_out[3][2];

  double u = scale;

  /* --------------------
     Define the six basic circles
     -------------------- */

  Circle center(0, 0, u * v.center_radius);

  for(int i = 0; i < 3; i++)
    {
      double theta = 120 * i;

      outer[i] = Circle(u * sin(deg2rad(theta)) * v.outer_offset,
			u * cos(deg2rad(theta)) * v.outer_offset,
			u * v.outer_radius);

      inner[i] = Circle(u * sin(deg2rad(theta)) * v.inner_offset,
			u * cos(deg2rad(theta)) * v.inner_offset,
			u * v.inner_radius);
    }
  /* --------------------
     Intersect the circles (inner/outer) & (outer/outer)
     -------------------- */

  for (int i = 0; i < 3; i++)
    {
      int j = (i+1)%3;
      outer[i].intersect(inner[i], in_out[i][0], in_out[i][1]);
      outer[i].intersect(outer[j], out_out[i][0], out_out[i][1]);
    }

  /* --------------------
     Trefoil
     -------------------- */

  cairo_set_line_width(cr, v.line_width);
  
  if (v.debug > 2)
    {
      cairo_set_line_width(cr, 1);
      cairo_set_source_rgb(cr, 0, 0, 1);
      for (int i = 0; i < 6; i++)
	{
	  cairo_save (cr);
	  cairo_move_to(cr, 0, 0);
	  cairo_rotate(cr, deg2rad(30 + i * 60));
	  cairo_line_to(cr, u * (v.outer_offset + v.outer_radius), 0);
	  cairo_stroke(cr);
	  cairo_restore(cr);
	}
      for (int i = 0; i < 3; i++)
	{
	  int j = (i+1)%3;
	  cairo_set_source_rgb(cr, 1, 0, 0);
	  inner[i].draw(cr);
	  inner[j].draw(cr);
	  cairo_stroke(cr);
	  Line(Point(0.0, 0.0), in_out[i][0]).draw(cr);
	  Line(Point(0.0, 0.0), out_out[i][0]).draw(cr);
	  Line(Point(0.0, 0.0), in_out[i][1]).draw(cr);
	  cairo_set_source_rgb(cr, 1, 1, 0);
	  outer[i].draw(cr);
	  outer[j].draw(cr);
	  cairo_stroke(cr);
	}
      cairo_restore(cr);
    }
    
  for (int i = 0; i < 3; i++)
    {
      int j = (i+1)%3;

      /* l0 - from center to inner first circle */

      Line l0 = Line(Point(0,0), inner[i].c);

      /* l3 - from center to second inner circle */

      Line l3 = Line(Point(0,0), inner[j].c);

      /* shift lines around their normals */

      l0.translate(u * v.center_offset);
      l3.translate(-u * v.center_offset);

      /* Intersect line with inner circle */

      Line l1; /* line_left_inner */
      inner[i].intersect(l0, l1.p[0], l1.p[1]);

      
      double ang_inner_l_start = (l1.p[1] - inner[i].c).polar();
      double ang_inner_l_end = (in_out[i][1] - inner[i].c).polar();

      double ang_outer_l_start = (in_out[i][1] - outer[i].c).polar();
      double ang_outer_l_end = (out_out[i][0] - outer[i].c).polar();
      double ang_outer_r_start = (out_out[i][0] - outer[j].c).polar();
      double ang_outer_r_end = (in_out[j][0] - outer[j].c).polar();
      double ang_inner_r_start = (in_out[j][0] - inner[j].c).polar();

      Line l4; /* line_right_center_inner */

      inner[j].intersect(l3, l4.p[0], l4.p[1]);

      double ang_inner_r_end = (l4.p[1] - inner[j].c).polar();

      Line l5; /* center circle and right offset */
      center.intersect(l3, l5.p[0], l5.p[1]);
      double center_start = l5.p[0].polar();

      Line l2; /* center cirle and left offset */
      center.intersect(l0, l2.p[0], l2.p[1]);

      double center_end = l2.p[0].polar();

      cairo_new_path(cr);

      /* outside right hand side arc of horn */
      cairo_arc_negative(cr, double(outer[j].c.x), double(outer[j].c.y), outer[j].r, ang_outer_r_start, ang_outer_r_end);

      /* inside right hand side arc of horn */
      cairo_arc(cr, double(inner[j].c.x), double(inner[j].c.y), inner[j].r, ang_inner_r_start, ang_inner_r_end);

      /* Center arc - the lines between the arcs are implicit in the path */
      cairo_arc(cr, 0.0, 0.0, v.center_radius * u, center_start, center_end);

      /* inside arc of horn */
      cairo_arc(cr, double(inner[i].c.x), double(inner[i].c.y), inner[i].r, ang_inner_l_start, ang_inner_l_end);

      /* outside arc of horn */
      cairo_arc_negative(cr, double(outer[i].c.x), double(outer[i].c.y), outer[i].r, ang_outer_l_start, ang_outer_l_end);

      cairo_close_path(cr);
      cairo_set_source_rgba(cr, v.fill[0], v.fill[1], v.fill[2], v.fill[3]);
      cairo_fill_preserve(cr);
      cairo_set_source_rgba(cr, v.stroke[0], v.stroke[1], v.stroke[2], v.stroke[3]);
      cairo_stroke(cr);
      
      //      path = cairo_copy_path(cr);
    }


  /* --------------------
     Ring
     -------------------- */

  if (v.debug)
    {
      printf(" ---- Ring ----\n");
    }
  
  for (int i = 0; i < 3; i++)
    {
      Circle inner_small = inner[i];
      inner_small.r -= u * v.ring_offset;

      Circle ring_outer(0, 0, v.ring_outer);
      ring_outer.scale(scale);
      Circle ring_inner(0, 0, v.ring_inner);
      ring_inner.scale(scale);

      cairo_new_path(cr);

      Line l1;
      ring_inner.intersect(inner_small, l1.p[0], l1.p[1]);
      Line l2;
      ring_outer.intersect(inner_small, l2.p[0], l2.p[1]);

      cairo_arc_negative(cr, double(ring_inner.c.x), double(ring_inner.c.y),
			 ring_inner.r, l1.p[0].polar(), l1.p[1].polar());

      cairo_arc(cr, double(inner_small.c.x), double(inner_small.c.y), inner_small.r,
		(l1.p[1] - inner_small.c).polar(),
		(l2.p[1] - inner_small.c).polar());

      cairo_arc(cr, double(ring_outer.c.x), double(ring_outer.c.y), ring_outer.r, l2.p[1].polar(), l2.p[0].polar());

      cairo_arc(cr, double(inner_small.c.x), double(inner_small.c.y), inner_small.r,
		(l2.p[0] - inner_small.c).polar(),
		(l1.p[0] - inner_small.c).polar());
      cairo_close_path(cr);
      cairo_set_source_rgb(cr, v.fill[0], v.fill[1], v.fill[2]);
      cairo_fill_preserve(cr);
      cairo_set_source_rgb(cr, v.stroke[0], v.stroke[1], v.stroke[2]);
      cairo_stroke(cr);
    }      

  cairo_restore(cr);
}

/* ----------------------------------------------------------------------
   --
   -- bio_cairo_3
   --
   ---------------------------------------------------------------------- */

void bio_cairo_3(cairo_t *cr, bio_t &bio)
{
  // Fill background
  cairo_rectangle(cr, 0, 0, bio.width, bio.height);
  cairo_set_source_rgb(cr, 0, 1, 0);
  //  cairo_fill(cr);

  // Center, flip vertical axis and scale
  
  cairo_translate(cr, bio.width/2, bio.height/2);
  cairo_scale(cr, 1.0, -1.0);
  cairo_rotate(cr, deg2rad(bio.theta));
  
  // Fix up line width to counter scale
  
  cairo_set_line_width(cr, 0.5);

  // Sizing based on "official" construction

  double scale = bio.width > bio.height ? bio.height : bio.width / 60.0;
  
  double a = scale;
  double b = 3.5 * a;
  double c = 4.0 * a;
  double d = 6.0 * a;
  double e = 11.0 * a;
  double f = 15.0 * a;
  double g = 21.0 * a;
  double h = 30.0 * a;

  cairo_path_t *trefoil_path;
  cairo_path_t* ring_path;

  {  
      cairo_new_path(cr);

      // Create lower trefoil starting from the center circle (radius d/2)

      Circle center_circle(Point(0, 0), d/2);

      // Line at 210°
      
      Line center_210(Point(0, 0), Point(1,0).rotate(deg2rad(210)));
      center_210.translate(-a/2);

      // Create left and right circles
      
      Circle inner_left(Point(f, 0).rotate(deg2rad(210)), g/2);
      Circle outer_left(Point(e, 0).rotate(deg2rad(210)), h/2);
      Circle inner_right(Point(f, 0).rotate(deg2rad(330)), g/2);
      Circle outer_right(Point(e, 0).rotate(deg2rad(330)), h/2);
      
      Point p_center_left_inner[2];
      center_circle.intersect(center_210, p_center_left_inner[0], p_center_left_inner[1]);
      
      Point p_inner_left[2];
      inner_left.intersect(center_210, p_inner_left[0], p_inner_left[1]);
      
      Line outer_left_bar(Point(0,0), Point(e+h, 0).rotate(deg2rad(210)));
      outer_left_bar.translate(-c/2);
      
      Point outer_left_bar_inner[2];
      Point outer_left_bar_outer[2];
      
      inner_left.intersect(outer_left_bar, outer_left_bar_inner[0], outer_left_bar_inner[1]);
      outer_left.intersect(outer_left_bar, outer_left_bar_outer[0], outer_left_bar_outer[1]);
      
      Line outer_middle(Point(0,0), Point(e+f, 0).rotate(deg2rad(-90)));
      Point middle[2];
      outer_left.intersect(outer_middle, middle[0], middle[1]);
      
      Line outer_right_bar(Point(0,0), Point(e+h, 0).rotate(deg2rad(330)));
      outer_right_bar.translate(c/2);
      
      Point outer_right_bar_outer[2];
      Point outer_right_bar_inner[2];
      outer_right.intersect(outer_right_bar, outer_right_bar_outer[0], outer_right_bar_outer[1]);
      inner_right.intersect(outer_right_bar, outer_right_bar_inner[0], outer_right_bar_inner[1]);
      
      Line inner_right_bar(Point(0,0), Point(e, 0).rotate(deg2rad(330)));
      inner_right_bar.translate(a/2);
      
      Point inner_right_bar_outer[2];
      Point inner_right_bar_inner[2];
      inner_right.intersect(inner_right_bar, inner_right_bar_outer[0], inner_right_bar_outer[1]);
      center_circle.intersect(inner_right_bar, inner_right_bar_inner[0], inner_right_bar_inner[1]);

      // Actual drawing
      
      cairo_set_source_rgb(cr, 1, 0, 0);
      cairo_arc_negative(cr, 0, 0, d/2, deg2rad(-90), p_center_left_inner[0].polar());
      cairo_line_to(cr, p_inner_left[1].x, p_inner_left[1].y);
      
      cairo_arc_negative(cr, inner_left.c.x, inner_left.c.y, inner_left.r,
			 (p_inner_left[1]-inner_left.c).polar(),
			 (outer_left_bar_inner[0]-inner_left.c).polar());
      
      cairo_line_to(cr, outer_left_bar_outer[0].x, outer_left_bar_outer[0].y);
      
      cairo_arc(cr, outer_left.c.x, outer_left.c.y, outer_left.r,
		(outer_left_bar_outer[0]-outer_left.c).polar(),
		(middle[0]-outer_left.c).polar());
      
      cairo_arc(cr, outer_right.c.x, outer_right.c.y, outer_right.r,
		(middle[0]-outer_right.c).polar(),
		(outer_right_bar_outer[0]-outer_right.c).polar());
      
      cairo_line_to(cr, outer_right_bar_inner[0].x, outer_right_bar_inner[0].y);
      
      cairo_arc_negative(cr, inner_right.c.x, inner_right.c.y, inner_right.r,
			 (outer_right_bar_inner[0]-inner_right.c).polar(),
			 (inner_right_bar_outer[1]-inner_right.c).polar());
      
      cairo_line_to(cr, inner_right_bar_inner[0].x, inner_right_bar_inner[0].y);
      
      cairo_arc_negative(cr, center_circle.c.x, center_circle.c.y, center_circle.r,
			 inner_right_bar_inner[0].polar(),
			 deg2rad(270));
      cairo_close_path(cr);
      trefoil_path = cairo_copy_path(cr);
      
      //      cairo_set_source_rgb(cr, 1, 1, 0);
      //      cairo_fill_preserve(cr);
      //      cairo_set_source_rgb(cr, 0, 0, 0);
      //      cairo_stroke(cr);

      /* Ring */

      Circle ring_inner(Point(0,0), e-a);
      Circle ring_outer(Point(0,0), e-a+b);
      Circle inner(Point(0, f), g/2 - a);
      
      Point p_ring_inner[2];
      Point p_ring_outer[2];
      
      ring_inner.intersect(inner, p_ring_inner[0], p_ring_inner[1]);
      ring_outer.intersect(inner, p_ring_outer[0], p_ring_outer[1]);


      cairo_new_path(cr);
      cairo_move_to(cr, 0, e-a);
      
      cairo_arc(cr, ring_inner.c.x, ring_inner.c.y, ring_inner.r,
		deg2rad(90),
		p_ring_inner[0].polar());
      
      cairo_arc_negative(cr, inner.c.x, inner.c.y, inner.r,
			 (p_ring_inner[0]-inner.c).polar(),
			 (p_ring_outer[0]-inner.c).polar());
      
      cairo_arc_negative(cr, ring_outer.c.x, ring_outer.c.y, ring_outer.r,
			 p_ring_outer[0].polar(),
			 p_ring_outer[1].polar());
      
      cairo_arc_negative(cr, inner.c.x, inner.c.y, inner.r,
			 (p_ring_outer[1]-inner.c).polar(),
			 (p_ring_inner[1]-inner.c).polar());
      
      
      cairo_arc(cr, ring_inner.c.x, ring_inner.c.y, ring_inner.r,
		p_ring_inner[1].polar(),
		deg2rad(90));      
      cairo_close_path(cr);
      ring_path = cairo_copy_path(cr);
    }

  cairo_new_path(cr);
  for (int i = 0; i < 3; i++)
    {
      cairo_append_path(cr, trefoil_path);
      
      // Fill & Stroke
      
      //	cairo_set_source_rgb(cr, 1, 1, 0);
      //	cairo_fill_preserve(cr);
      //	cairo_set_source_rgb(cr, 0, 0, 0);
      //	cairo_stroke(cr);
      
      cairo_pattern_t *p = cairo_pattern_create_linear(0, 0, 0, -(e+h));
      cairo_pattern_add_color_stop_rgba(p, 0, 0, 1, 0, 1);
      cairo_pattern_add_color_stop_rgba(p, 1, 0, 0, 1, 1);
      cairo_set_source(cr, p);
      
      cairo_fill_preserve(cr);
      cairo_set_source_rgba(cr, 0, 0, 0, 1); // v.stroke[0], v.stroke[1], v.stroke[2], v.stroke[3]);
      cairo_stroke(cr);

      p = cairo_pattern_create_linear(0, 0, 0, -e+h);
      cairo_pattern_add_color_stop_rgba(p, 0, 0, 1, 0, 1);
      cairo_pattern_add_color_stop_rgba(p, 1, 0, 0, 1, 1);
      cairo_set_source(cr, p);
      cairo_append_path(cr, ring_path);
      cairo_set_source(cr, p);
      cairo_fill_preserve(cr);
      cairo_set_source_rgba(cr, 0, 0, 0, 1);
      cairo_stroke(cr);

      
      // Rotate
      
      cairo_rotate(cr, deg2rad(120));
    }
  
}  
		  


/* ----------------------------------------------------------------------
--
-- bio_init
--
---------------------------------------------------------------------- */

void bio_init(int argc, char *argv[], bio_t &v)
{
  v.theta = 0.0;
  v.inner_radius = 11.5;
  v.inner_offset = 17.6;
  v.outer_radius = 14.0;
  v.outer_offset = 15.0;
  v.center_radius = 4.0;
  v.center_offset = 1.0;

  v.ring_outer = 15.0;
  v.ring_inner = 11.0;
  v.ring_offset = 1.0;
  
  v.fill[0] = 1.0;
  v.fill[1] = 1.0;
  v.fill[2] = 0.0;
  v.fill[3] = 1.0;
  v.stroke[0] = 0.0;
  v.stroke[1] = 0.0;
  v.stroke[2] = 0.0;
  v.stroke[3] = 1.0;
  v.line_width = 0.5;
  v.width = 800;
  v.height = 800;
  v.scale = 5.0;
  v.debug = 0;
  v.rotation = 0;
  v.horn_path = 0;
  v.ring_path = 0;

  if (argc > 1)
    {
      const char *f = argv[1];
      FILE *stream;
      std::string ss;
      if ((stream = fopen(f, "r")))
	{
	  char buf[1024];
	  int len;
	  while ((len = fread(buf, sizeof(char), sizeof(buf), stream)))
	    ss.append(buf, len);
	  fclose(stream);
	}
      json_object *js = json_tokener_parse(ss.c_str());
      assert(js != 0);
      assert(json_object_is_type(js, json_type_object));
      
      json_object *t;
      if(json_object_object_get_ex(js, "line_width", &t)) v.line_width = json_object_get_double(t);
      if(json_object_object_get_ex(js, "scale", &t)) v.scale = json_object_get_double(t);
      if(json_object_object_get_ex(js, "width", &t)) v.width = json_object_get_double(t);
      if(json_object_object_get_ex(js, "height", &t)) v.height = json_object_get_double(t);
      if(json_object_object_get_ex(js, "inner_radius", &t)) v.inner_radius = json_object_get_double(t);
      if(json_object_object_get_ex(js, "inner_offset", &t)) v.inner_offset = json_object_get_double(t);
      if(json_object_object_get_ex(js, "outer_radius", &t)) v.outer_radius = json_object_get_double(t);
      if(json_object_object_get_ex(js, "outer_offset", &t)) v.outer_offset = json_object_get_double(t);
      if(json_object_object_get_ex(js, "center_radius", &t)) v.center_radius = json_object_get_double(t);
      if(json_object_object_get_ex(js, "center_offset", &t)) v.center_offset = json_object_get_double(t);
      if(json_object_object_get_ex(js, "ring_inner", &t)) v.ring_inner = json_object_get_double(t);
      if(json_object_object_get_ex(js, "ring_outer", &t)) v.ring_outer = json_object_get_double(t);
      if(json_object_object_get_ex(js, "ring_offset", &t)) v.ring_offset = json_object_get_double(t);
      if(json_object_object_get_ex(js, "fill", &t))
	{
	  const char *str = json_object_get_string(t);
	  unsigned char f[4];
	  sscanf(str, "#%2x%2x%2x%2x", &f[0], &f[1], &f[2], &f[3]);
	  for (int k = 0; k < 4; k++) {
	    v.fill[k] = f[k]/256.0;
	  }
	}
      if(json_object_object_get_ex(js, "stroke", &t))
	{
	  const char *str = json_object_get_string(t);
	  unsigned char f[4];
	  sscanf(str, "#%2x%2x%2x%2x", &f[0], &f[1], &f[2], &f[3]);
	  for (int k = 0; k < 4; k++) {
	    v.stroke[k] = f[k]/256.0;
	  }
	}
    }
}

bio_t::bio_t(int argc, char** argv)
{
  bio_init(argc, argv, *this);
}

void bio_t::cairo(cairo_t *cr)
{
  bio_cairo_3(cr, *this);
}
