#include <iostream>
#include <math.h>
#include <stdio.h>
#include <cairomm/cairomm.h>
#include "geometry.h"

class cpoint_t : public point_t {
public:
  cpoint_t() : point_t() {};
  cpoint_t(double d1, double d2) : point_t(d1, d2) {};
  cpoint_t(point_t&p) : point_t(p) {};
  cpoint_t operator+(const point_t &p)
  {
    cpoint_t r(x + p.x, y + p.y);
    return r;
  }
  void move_to(Cairo::RefPtr<Cairo::Context> cr) { cr->move_to(x, y); }
  void line_to(Cairo::RefPtr<Cairo::Context> cr) { cr->line_to(x, y); }
  void label(Cairo::RefPtr<Cairo::Context> cr, double x_off, double y_off, const char* s)
  {
    cr->move_to(x + x_off, y + y_off);
    cr->save();
    cr->scale(1,-1);
    cr->show_text(s);
    cr->restore();
  }
};

/* ----------------------------------------------------------------------

   euro

   ----------------------------------------------------------------------*/

void euro(Cairo::RefPtr<Cairo::ImageSurface> surface, bool original)
{
  // outer radius and width

  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);

  // Center and invert y-axis  
  cr->translate(400, 400);
  cr->scale(1.5, -1.5);
  cr->set_line_width(1.5);

  double outer_radius = 200;
  double width = 20;
  double inner_radius = outer_radius - width;

  point_t origin(0, 0);
  point_t unit(1, 0);
  point_t a1 = (unit*inner_radius).rotate(deg2rad(45));
  point_t a5 = (unit*inner_radius).rotate(deg2rad(-45));
  cpoint_t a2(0.0, -outer_radius);

  circle_t outer(0, 0, outer_radius);
  circle_t inner(0, 0, inner_radius);

  line_t lines[6];
  point_t x[24];

  
  // Subtract a2 from a1 to get vector from the origin
  
  point_t a3 = a1 - a2;
  lines[0] = line_t(a2, a1);
  point_t a4 = point_t(a5.x, 0);
  lines[1] = line_t(a5, a4);
  //  point_t a4(a5.x, 0);
  //  line_t a5_unit(a5, a4);
  
  a1.print("a1 ");
  a2.print("a2 ");
  a3.print("a3 ");
  a5.print("a5 ");

  for (int i = 0; i < 4; i++)
    {
      lines[i+2].p[0].x = -outer_radius;
      lines[i+2].p[1].x = +outer_radius;
      lines[i+2].p[0].y = (-1.5 + i) * width;
      lines[i+2].p[1].y = (-1.5 + i) * width;
    }

  for (int i = 0; i < 6; i++)
    { 
      inner.intersect(lines[i], x[4*i+0], x[4*i+1]);
      outer.intersect(lines[i], x[4*i+2], x[4*i+3]);
    }
  //  inner.intersect(lines[1], x[4], x[5]);
  //  outer.intersect(lines[1], x[6], x[7]);
  
  x[0].print("x0 ");
  x[1].print("x1 ");
  x[2].print("x2 ");
  x[3].print("x3 ");
  x[4].print("x4 ");
  x[5].print("x5 ");
  x[6].print("x6 ");
  x[7].print("x7 ");

  // Get angle (tan theta = opposite / adjacent)
  
  double theta = a3.polar();
  double phi = M_PI/2.0 - theta;
  
  std::cout << "theta = " << rad2deg(theta) << "\n";


  double l0 = width / tan(theta); // tanθ = o/a => o = a * tanθ
  cpoint_t a0(l0, width);
      

  cpoint_t bar[8];
  // lower bar
      
  double l7 = outer_radius - 1.5 * width; // vertical height lower edge
  //  double l8 = outer_radius - 0.5 * width;  // vertical hieght upper edge
  double l9 = l7 / tan(theta);
  //  double l10 = tan(theta) * l8;
  
  bar[0] = cpoint_t(-outer_radius - l9, -1.5*width);
  bar[1] = cpoint_t(l9, -1.5*width);
  bar[2] = bar[1] + a0;
  bar[3] = bar[0] + a0;
  
  double l11 = outer_radius + 0.5 * width;
  //  double l12 = outer_radius + 1.5 * width;
  double l13 = l11 / tan(theta);
  //  double l14 = tan(theta) * l12;
  
  bar[4] = cpoint_t(-outer_radius - l9, 0.5*width);
  bar[5] = cpoint_t(l13, 0.5*width);
  bar[6] = bar[5] + a0;
  bar[7] = bar[4] + a0;
  
  if (original)
    {
      // Angle that is subtracted from lower outer arc
      double A = (1.0/4.0 * M_PI) - asin(sin(M_PI * 3.0/4.0) * (inner_radius/outer_radius));
      
      // Angle added to upper outer arc
      double B = M_PI - 3.0/4.0*M_PI - phi - asin((inner_radius/outer_radius) * sin(3.0/4.0*M_PI + phi));
      
      std::cout << "A = " << A * 180 / M_PI << "\n";
      std::cout << "B = " << B * 180 / M_PI << "\n";
      
      /* --------------------
	 Create ring
	 -------------------- */
      
      cr->set_source_rgb(0,1,0);  
      
      cr->arc(0, 0, inner_radius, M_PI/4, 7 * M_PI/4);   // Inside arc
      cr->arc_negative(0, 0, outer_radius, 7 * M_PI/4 - A, M_PI/4 + B); // Outside arc
      cr->close_path();
      
      
      bar[0].move_to(cr);
      bar[1].line_to(cr);
      bar[2].line_to(cr);
      bar[3].line_to(cr);
      cr->close_path();
      
      bar[4].move_to(cr);
      bar[5].line_to(cr);
      bar[6].line_to(cr);
      bar[7].line_to(cr);
      cr->close_path();
      cr->fill();
    }

  
  /* --------------------
     Construction
     -------------------- */

  cpoint_t c_origin(origin);
  cpoint_t c1(a1);
  cpoint_t c5(a5);
  
  //  cr->set_source_rgb(1,0,0);
  //  c_origin.move_to(cr);  
  //  a2.line_to(cr);
  //  c1.line_to(cr);
  //  cr->stroke();

  //  c_origin.move_to(cr);
  //  c1.line_to(cr);
  
  //  c_origin.move_to(cr);
  //  c5.line_to(cr);
  //  cr->stroke();
  
  
  cpoint_t cx[8];
  for (int i = 0; i < 8; i++)
    cx[i] = x[i];
  
      
  cx[0].move_to(cr);
  cx[2].line_to(cr);
  cr->arc(0, 0, outer_radius, x[2].polar(), x[23].polar());
  cr->line_to(bar[7].x, bar[7].y);
  cr->line_to(bar[4].x, bar[4].y);
  cr->line_to(x[19].x, x[19].y);
  cr->arc(0, 0, outer_radius, x[19].polar(), x[15].polar());
  
  cr->line_to(bar[3].x, bar[3].y);
  cr->line_to(bar[0].x, bar[0].y);
  cr->line_to(x[11].x, x[11].y);
  cr->arc(0, 0, outer_radius, x[11].polar(), x[7].polar());
  cr->line_to(x[5].x, x[5].y);
  
  cr->arc_negative(0, 0, inner_radius, x[5].polar(), x[9].polar());

  cr->line_to(bar[1].x, bar[1].y);
  cr->line_to(bar[2].x, bar[2].y);
  cr->line_to(x[13].x, x[13].y);
  
  cr->arc_negative(0, 0, inner_radius, x[13].polar(), x[17].polar());

  cr->line_to(bar[5].x, bar[5].y);
  cr->line_to(bar[6].x, bar[6].y);
  cr->line_to(x[21].x, x[21].y);

  cr->arc_negative(0, 0, inner_radius, x[21].polar(), x[0].polar());
  cr->close_path();
  cr->set_source_rgb(0,1,0);
  cr->fill_preserve();
  cr->set_source_rgb(0,0,0);
  cr->stroke();

  cr->set_source_rgb(0, 0, 0);

  for (int i = 0; i < 24; i++)
    {
      char label[4];
      cr->move_to(x[i].x, x[i].y);
      cr->save();
      cr->scale(1, -1);
      sprintf(label, "X%02d", i);
      cr->show_text(label);
      cr->restore();
    }
  
  cr->set_source_rgb(0,0,0);
  cr->stroke();
      
  bar[0].label(cr, 3.0, -10.0, "BAR[0]");
  bar[1].label(cr, 3.0, -10.0, "BAR[1]");
  bar[2].label(cr, 3.0, -10.0, "BAR[2]");
  bar[3].label(cr, 3.0, -10.0, "BAR[3]");
  bar[4].label(cr, 3, -3, "BAR[4]");
  bar[5].label(cr, 3, -3, "BAR[5]");
  bar[6].label(cr, 3, -3, "BAR[6]");
  bar[7].label(cr, 3, -3, "BAR[7]");
}

/* ----------------------------------------------------------------------

   main

   ---------------------------------------------------------------------- */

int main()
{
  Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, 800, 800);
  euro(surface, false);
  surface->write_to_png("image.png");
}
