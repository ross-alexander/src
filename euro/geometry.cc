#include <cstdio>
#include <cmath>
#include <cassert>
#include <iostream>

#include "geometry.h"

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
-- point_t
--
---------------------------------------------------------------------- */

point_t::point_t() { x = y = 0.0; }
point_t::point_t(const point_t& p) { x = p.x; y = p.y; }
point_t::point_t(double px, double py)
{
  x = px;
  y = py;
}
point_t point_t::operator-(const point_t& p)
{
  point_t r(x-p.x, y-p.y);
  return r;
}
point_t point_t::operator*(const double l)
{
  return point_t(x*l, y*l);
}

point_t& point_t::operator*=(const double l)
{
  x *= l;
  y *= l;
  return *this;
}

point_t& point_t::scale(const double l)
{
  x *= l;
  y *= l;
  return *this;
}

point_t& point_t::operator+=(const point_t &p)
{
  x += p.x;
  y += p.y;
  return *this;
}
point_t& point_t::rotate(double theta)
{
  //    theta = deg2rad(theta);
  double xr = cos(theta) * x - sin(theta) * y;
  double yr = sin(theta) * x + cos(theta) * y;
  x = xr;
  y = yr;
  return *this;
}
double point_t::polar()
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
  point_t point_t::normal()
  {
    double l = sqrt(x * x + y * y);
    return point_t(x/l, y/l);
  }


void point_t::print(const char*s) { printf("%s[%8.2f %8.2f]\n", s ? s : "", x, y); };

/* ----------------------------------------------------------------------
--
-- Line
--
---------------------------------------------------------------------- */

line_t::line_t() {};
line_t::line_t(point_t& p1, point_t& p2) { p[0] = p1; p[1] = p2; }
void line_t::translate(double s)
{
  point_t x = (p[1] - p[0]).normal();
  x.rotate(-90);
  p[0] += x * s;
  p[1] += x * s;
}
void line_t::print(const char* s = "")
{
  printf("line_t %s (%f, %f) -> (%f, %f)\n", s, p[0].x, p[0].y, p[1].x, p[1].y);
}


/* ----------------------------------------------------------------------
--
-- circle_t
--
---------------------------------------------------------------------- */


circle_t::circle_t() { r = 0.0; }
circle_t::circle_t(double px, double py, double pr)
{
  c.x = px;
  c.y = py;
  r = pr;
};

circle_t::circle_t(point_t p, double rad)
{
  c = p;
  r = rad;
}

circle_t& circle_t::scale(double s)
{
  c.scale(s);
  r *= s;
  return *this;
}


bool circle_t::within(double x1, double y1, double x2, double y2, double x, double y) {
    double d1 = sqrt(square(x2 - x1) + square(y2 - y1));    // distance between end-points
    double d2 = sqrt(square(x - x1) + square(y - y1));      // distance from point to one end
    double d3 = sqrt(square(x2 - x) + square(y2 - y));      // distance from point to other end
    double delta = d1 - d2 - d3;
    return fabs(delta) < eps;   // true if delta is less than a small tolerance
}
  
int circle_t::rxy(double x1, double y1, double x2, double y2, double x, double y, bool segment) {
    if (!segment || circle_t::within(x1, y1, x2, y2, x, y)) {
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
void circle_t::isec(point_t& p1, point_t& p2, point_t& cp, double r, bool segment)
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

  
int circle_t::intersect(const line_t& l, point_t &p1, point_t &p2)
  {
    double x1, y1, x2, y2;
    int res;
    assert((res = circle_line_intersection(l.p[0].x, l.p[0].y,
					   l.p[1].x, l.p[1].y,
					   c.x, c.y, r,
					   &x1, &y1, &x2, &y2)) > 0);
    p1.x = x1; p1.y = y1;
    p2.x = x2; p2.y = y2;
    return res;
  }

int circle_t::intersect(const circle_t &cir, point_t &p1, point_t &p2)
{
  int res;
  assert((res = circle_circle_intersection(c.x, c.y, r,
					   cir.c.x, cir.c.y, cir.r,
					   &p1.x, &p1.y, &p2.x, &p2.y)) > 0);
  return res;
};

