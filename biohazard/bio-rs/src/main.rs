/* ----------------------------------------------------------------------
--
-- bio-rs
--
-- 2026-02-22: Initial version
--
---------------------------------------------------------------------- */

#![allow(non_camel_case_types)]
#![allow(unused_imports)]
#![allow(dead_code)]
#![allow(unused_variables)]

use std::error::Error;
use std::f64;
use std::ops::*;
use std::f64::consts::PI;
use std::fmt;
use cairo::{Context, Format, ImageSurface, Surface, PdfSurface, SvgSurface};

/* --------------------
Collect feature sizes together
-------------------- */

struct Params {
   center_radius : f64,
   outer_offset : f64,
   outer_radius : f64,
   inner_offset : f64,
   inner_radius : f64,
   ring_inner_radius : f64,
   ring_outer_radius : f64,
   horn_gap : f64,
   circle_gap : f64,
   width : f64,
   height : f64,
}

/* --------------------
Everything is based on a Point.  This needs Clone to avoid a move
when using it as an function argument.
-------------------- */

#[derive(Copy, Clone)]
struct Point {
    x: f64,
    y: f64
}

/* --------------------
A line is just two points.  Use array rather named variables
-------------------- */

struct Line {
    p: [Point; 2],
}

/* --------------------
Circle doesn't need clone as it passed as reference to arguments
-------------------- */

struct Circle {
    center: Point,
    radius: f64
}

/* ----------------------------------------------------------------------
--
-- helper functions
--
---------------------------------------------------------------------- */

fn rad2deg(theta: f64) -> f64 {
    return theta * 180.0 / PI;
}

fn deg2rad(theta: f64) -> f64 {
    return theta / 180.0 * PI;
}


fn circle_circle_intersection(x0: f64, y0:f64, r0: f64, x1: f64, y1: f64, r1: f64) -> (Option<(f64, f64)>,
                                                                                       Option<(f64, f64)>)
{
    let dx = x1 - x0;
    let dy = y1 - y0;

    let d = dx.hypot(dy); // Suggested by Keith Briggs

    if d > (r0 + r1)
    {
    /* no solution. circles do not intersect. */
        return (None, None);
    }

    if d - (r0 - r1).abs() < 0.0
    {
        /* no solution. one circle is contained in the other */
        return  (None, None);
    }

      /* Determine the distance from point 0 to point 2. */

  let a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0 * d) ;

  /* Determine the coordinates of point 2. */
  
  let x2 = x0 + (dx * a/d);
  let y2 = y0 + (dy * a/d);

  /* Determine the distance from point 2 to either of the
   * intersection points.
   */
  
  let h = ((r0*r0) - (a*a)).sqrt();

  let rx = -dy * (h/d);
  let ry = dx * (h/d);

    /* Determine the absolute intersection points. */

  let xi = x2 + rx;
  let xi_prime = x2 - rx;
  let yi = y2 + ry;
  let yi_prime = y2 - ry;

    
    return (Some((xi, yi)), Some((xi_prime, yi_prime)));
}


fn circle_line_intersection (x1: f64, y1: f64,
			     x2: f64, y2: f64,
                             x3: f64, y3: f64, r: f64) -> (Option<(f64, f64)>, Option<(f64, f64)>)
{

  // x1,y1 P1 coordinates (point of line)
  // x2,y2 P2 coordinates (point of line)
  // x3,y3,r  P3 coordinates and radius (circle)
  // x,y      intersection coordinates

  // This function returns a pointer array which first index indicates
  // the number of intersection point, followed by coordinate pairs.
  
  let a =  (x2 - x1)*(x2 - x1) + (y2 - y1) * (y2 - y1);
  let b =  2.0 * ((x2 - x1)*(x1 - x3) + (y2 - y1)*(y1 - y3));
  let c =  x3*x3 + y3*y3 + x1*x1 + y1*y1 - 2.0 * (x3*x1 + y3*y1) - r*r;
  let i =  b * b - 4.0 * a * c;
  
  if i < 0.0
    {
      // no intersection
      return (None, None);
    }
  
  if i == 0.0
    {
      let mu = -b/(2.0 * a) ;
      let rx1 = x1 + mu*(x2-x1);
      let ry1 = y1 + mu*(y2-y1);
      return (Some((rx1, ry1)), None);
    }
  else if i > 0.0
    {
      // first intersection
      let mu1 = (-b + (b*b - 4.0 * a * c).sqrt()) / (2.0 * a);
      let rx1 = x1 + mu1*(x2-x1);
      let ry1 = y1 + mu1*(y2-y1);
      
      // second intersection
      let mu2 = (-b - (b*b - 4.0*a*c).sqrt()) / (2.0*a);
      let rx2 = x1 + mu2*(x2-x1);
      let ry2 = y1 + mu2*(y2-y1);
      return (Some((rx1, ry1)), Some((rx2, ry2)));
    }
    else
    {
        return (None, None);
    }
}

/* ----------------------------------------------------------------------
--
-- Point
--
---------------------------------------------------------------------- */

impl Point {
    fn new (x: f64, y: f64) -> Self {
        return Point {x: x, y: y};
    }
    fn desc(&self) -> String {
        let s: String = std::format!("{:6.2}, {:6.2}", self.x, self.y);
        return s;
    }
    fn metric(&self) -> f64 {
        return (self.x * self.x + self.y * self.y).sqrt();
    }
    fn polar(&self) -> f64 {
        return self.y.atan2(self.x);
    }
    fn rotate(&self, theta: f64) -> Self {
        return Self {x: theta.cos() * self.x - theta.sin() * self.y,
                     y: theta.sin() * self.x + theta.cos() * self.y}
    }
    fn scale(&self, s: f64) -> Self {
        return Self {x: self.x * s, y: self.y * s};
    }
    fn normal(&self) -> Self {
        let m = self.metric();
        return Self {x: self.x / m, y: self.y / m};
    }
}

impl Add for Point {
    type Output = Point;
    fn add(self, p: Point) -> Point {
        return Point::new(self.x + p.x, self.y + p.y);
    }
}
impl Sub for Point {
    type Output = Point;
    fn sub(self, p: Point) -> Point {
        return Point::new(self.x - p.x, self.y - p.y);
    }
}
impl fmt::Display for Point {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "({:7.2}, {:7.2})", self.x, self.y)
    }
}

/* ----------------------------------------------------------------------
--
-- Line
--
---------------------------------------------------------------------- */

impl Line {
    fn new(a: Point, b: Point) -> Self {
        return Line { p: [a, b] };
    }
    fn scale(&self, s: f64) -> Self {
      return Line {p: [Point {x: self.p[0].x * s, y: self.p[0].y * s},
		       Point {x: self.p[1].x * s, y: self.p[1].y * s}]};
    }
    fn translate(&self, s: f64) -> Self {
        let x = Point {x: self.p[1].x - self.p[0].x, y: self.p[1].y - self.p[0].y}.normal().rotate(deg2rad(90.0));
        return Line {p: [Point {x: self.p[0].x + x.x*s, y: self.p[0].y + x.y*s},
		         Point {x: self.p[1].x + x.x*s, y: self.p[1].y + x.y*s}]};
    }
    fn circle_intersect(&self, c: &Circle) -> (Point, Point)
    {
        return match circle_line_intersection(self.p[0].x, self.p[0].y,
                                              self.p[1].x, self.p[1].y,
                                              c.center.x, c.center.y, c.radius) {
            (Some((x1, y1)), Some((x2, y2))) => (Point::new(x1, y1), Point::new(x2, y2)),
            (Some((x1, y1)), None) => (Point::new(x1, y1), Point::new(0.0, 0.0)),
            (None, Some((x1, y1))) => (Point::new(0.0, 0.0), Point::new(x1, y1)),
            (None, None) => (Point::new(0.0, 0.0), Point::new(0.0, 0.0)),
        }
    }
}


/* ----------------------------------------------------------------------
--
-- circle
--
---------------------------------------------------------------------- */

impl Circle {
    fn circle_intersect(&self, c: &Circle) -> (Point, Point)
    {
        return match circle_circle_intersection(self.center.x, self.center.y, self.radius,
                                          c.center.x, c.center.y, c.radius) {
            (Some((x1, y1)), Some((x2, y2))) => (Point::new(x1, y1), Point::new(x2, y2)),
            (Some((x1, y1)), None) => (Point::new(x1, y1), Point::new(0.0, 0.0)),
            (None, Some((x1, y1))) => (Point::new(0.0, 0.0), Point::new(x1, y1)),
            (None, None) => (Point::new(0.0, 0.0), Point::new(0.0, 0.0)),
        }
    }
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

fn draw_surface(params: &Params, surface: &cairo::Surface) -> Result<(), Box<dyn Error>>
{
    let origin = Point {x: 0.0, y: 0.0 };
    let unit_point = Point {x: 1.0, y: 0.0 };
    let unit_point_210 = unit_point.rotate(deg2rad(210.0));
    let unit_point_330 = unit_point.rotate(deg2rad(330.0));
    
    // Do lower trefoil

    let left_outer = Circle {center : unit_point_210.scale(params.outer_offset), radius : params.outer_radius};
    let right_outer = Circle {center : unit_point_330.scale(params.outer_offset), radius : params.outer_radius};
    
    let left_inner = Circle {center : unit_point_210.scale(params.inner_offset), radius : params.inner_radius};
    let right_inner = Circle {center : unit_point_330.scale(params.inner_offset), radius : params.inner_radius};

    let center_circle = Circle {center : Point {x : 0.0, y : 0.0}, radius : params.center_radius};
    
    let left_horn_line = Line::new(origin, unit_point_210).scale(params.outer_offset + 2.0 * params.outer_radius).translate(params.horn_gap/2.0);
    let right_horn_line = Line::new(origin, unit_point_330).scale(params.outer_offset + 2.0 * params.outer_radius).translate(-params.horn_gap/2.0);

    let left_center_line = Line::new(origin, unit_point_210).scale(params.outer_offset + 2.0 * params.outer_radius).translate(params.circle_gap/2.0);
    let right_center_line = Line::new(origin, unit_point_330).scale(params.outer_offset + 2.0 * params.outer_radius).translate(-params.circle_gap/2.0);

    
    let (left_horn_outer,_) = left_horn_line.circle_intersect(&left_outer); // A
    let (_, outer_intersect) = left_outer.circle_intersect(&right_outer); // B
    let (right_horn_outer,_) = right_horn_line.circle_intersect(&right_outer); // C

    let (right_horn_inner,_) = right_horn_line.circle_intersect(&right_inner); // D
    let (_,right_inner_gap) = right_center_line.circle_intersect(&right_inner); // E
    let (right_circle_gap,_) = right_center_line.circle_intersect(&center_circle); // F
    let (left_circle_gap,_) = left_center_line.circle_intersect(&center_circle); // G

   let (_,left_inner_gap) = left_center_line.circle_intersect(&left_inner); // H
   let (left_horn_inner,_) = left_horn_line.circle_intersect(&left_inner); // I

   let ang_outer_l_start = (left_horn_outer - left_outer.center).polar();
   let ang_outer_l_end = (outer_intersect - left_outer.center).polar();

   let ang_outer_r_start = (outer_intersect - right_outer.center).polar();
   let ang_outer_r_end = (right_horn_outer - right_outer.center).polar();

   let ang_inner_r_start = (right_horn_inner - right_inner.center).polar();
   let ang_inner_r_end = (right_inner_gap - right_inner.center).polar();

   let ang_circle_start = right_circle_gap.polar();
   let ang_circle_end = left_circle_gap.polar();
   
   let ang_inner_l_start = (left_inner_gap - left_inner.center).polar();
   let ang_inner_l_end = (left_horn_inner - left_inner.center).polar();

    
    print!("{}\n", left_horn_outer);
    print!("{}\n", outer_intersect);
    print!("{}\n", right_horn_outer);
    print!("{}\n", right_horn_inner);
    print!("{}\n", right_inner_gap);
    print!("{}\n", right_circle_gap);
    print!("{}\n", left_circle_gap);
    print!("{}\n", left_inner_gap);
    print!("{}\n", left_horn_inner);

   /* --------------------
   -- Ring
   -------------------- */

    let ring_center = Circle {center: Point::new(0.0, params.inner_offset), radius: params.inner_radius - params.circle_gap};
    let ring_inner = Circle {center: origin, radius: params.ring_inner_radius};
    let ring_outer = Circle {center: origin, radius: params.ring_outer_radius};

    let (ring_inner_right, ring_inner_left) = ring_center.circle_intersect(&ring_inner); // -- J & K
    let (ring_outer_right, ring_outer_left) = ring_center.circle_intersect(&ring_outer); // L & M
    let inner_center = Point::new(0.0, params.inner_offset);

    /* --------------------
    -- Cairo
    -------------------- */
    
    let cr = Context::new(surface).expect("Could not create context");

    cr.translate(params.width/2.0, params.height/2.0);
    let scale = 4.0 / 3.0;
    cr.scale(scale, -scale);
    cr.select_font_face("URW Gothic L", cairo::FontSlant::Normal, cairo::FontWeight::Bold);
    cr.set_font_size(14.0);

    cr.new_path();
    cr.arc(left_outer.center.x, left_outer.center.y, left_outer.radius, ang_outer_l_start, ang_outer_l_end); //   -- A--B
    cr.arc(right_outer.center.x, right_outer.center.y, right_outer.radius, ang_outer_r_start, ang_outer_r_end); // -- B--C
    cr.line_to(right_horn_inner.x, right_horn_inner.y); // -- C-D
    cr.arc_negative(right_inner.center.x, right_inner.center.y, right_inner.radius, ang_inner_r_start, ang_inner_r_end); // -- D--E
    cr.line_to(right_inner_gap.x, right_inner_gap.y); // -- E--F
    cr.arc_negative(0.0, 0.0, params.center_radius, ang_circle_start, ang_circle_end); // -- F-G
    cr.line_to(left_inner_gap.x, left_inner_gap.y); // -- G-H
    cr.arc_negative(left_inner.center.x, left_inner.center.y, left_inner.radius, ang_inner_l_start, ang_inner_l_end); // -- H-I
    cr.close_path();
        
    let trefoil_path = cr.copy_path()?;

    cr.new_path();
    cr.move_to(ring_inner_left.x, ring_inner_left.y);
    cr.arc_negative(0.0, 0.0, params.ring_inner_radius, ring_inner_left.polar(), ring_inner_right.polar());
    cr.arc(0.0, params.inner_offset, params.inner_radius-params.circle_gap,
	   (ring_inner_right - inner_center).polar(),
	   (ring_outer_right - inner_center).polar());
    cr.arc(0.0, 0.0, params.ring_outer_radius, ring_outer_right.polar(), ring_outer_left.polar());
    cr.arc(0.0, params.inner_offset, params.inner_radius-params.circle_gap,
	   (ring_outer_left - inner_center).polar(),
	   (ring_inner_left - inner_center).polar());
    cr.close_path();
    let ring_path = cr.copy_path()?;

    cr.set_line_width(0.5);
    
    cr.new_path();
    for i in 0..3 {
        cr.save()?;
        cr.rotate(deg2rad(120.0) * i as f64);
        cr.append_path(&trefoil_path);
        cr.append_path(&ring_path);
        cr.restore()?;
    }
    cr.set_source_rgb(1.0, 0.0, 0.0);
    cr.fill_preserve()?;
    cr.set_source_rgb(0.0, 0.0, 0.0);
    cr.stroke()?;
    return Ok(());
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

fn main() -> Result<(), Box<dyn Error>> {
    let params = Params {
        center_radius : 30.0,
        outer_offset : 110.0,
        outer_radius : 150.0,
        inner_offset : 150.0,
        inner_radius : 105.0,
        ring_inner_radius : 100.0,
        ring_outer_radius : 135.0,
        horn_gap : 40.0,
        circle_gap : 10.0,
        width : 800.0,
        height : 800.0,
    };

    /* --------------------
    -- SVG
    -------------------- */
    
    let svg = SvgSurface::new(params.width as f64, params.height as f64, Some("output.svg")).expect("Could not create SVG surface");
    draw_surface(&params, &svg)?;

    /* --------------------
    -- PDF
    -------------------- */

    let pdf = PdfSurface::new(params.width as f64, params.height as f64, "output.pdf").expect("Could not create PDF surface");
    draw_surface(&params, &pdf)?;

    Ok(())
}
