#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gts.h>

int test1(int argc, char *argv[])
{
  double d = 0.0;

  if (argc > 1)
    {
      d = strtod(argv[1], NULL);
    }

  for (int i = 0; i < 6; i++)
    {
      double a1 = 0.0;
      double a2 = 0.0;
      double a3 = d;
      
      double b1 = cos(M_PI/3 * (i+0));
      double b2 = sin(M_PI/3 * (i+0));
      double b3 = 0.0;
      
      double c1 = cos(M_PI/3 * (i+1));
      double c2 = sin(M_PI/3 * (i+1));
      double c3 = 0.0;
      
      double d1 = b1 - a1;
      double d2 = b2 - a2;
      double d3 = b3 - a3;
      
      double e1 = c1 - a1;
      double e2 = c2 - a2;
      double e3 = c3 - a3;



      double f1 = d2 * e3 - d3 * e2;
      double f2 = d3 * e1 - d1 * e3;
      double f3 = d1 * e2 - d2 * e1;

      double f = sqrt(f1*f1+f2*f2+f3*f3);



      double g1 = cos(M_PI/3 * (i+0));
      double g2 = sin(M_PI/3 * (i+0));
      double g3 = -d;

      double h1 = cos(M_PI/3 * (i+1));
      double h2 = sin(M_PI/3 * (i+1));
      double h3 = -d;

      printf("d = %f %f %f\n", d1, d2, d3);
      printf("g = %f %f %f\n", g1, g2, g3);

      printf("e = %f %f %f\n", e1, e2, e3);
      printf("h = %f %f %f\n", h1, h2, h3);

      double i1 = sin(M_PI/3 * (i+0)) * -d - -d * sin(M_PI/3 * (i+1));

      double i2 = -d * cos(M_PI/3 * (i+1)) - cos(M_PI/3 * (i+0)) * -d;

      double i3 = cos(M_PI/3 * (i+0)) * sin(M_PI/3 * (i+1)) - sin(M_PI/3 * (i+0)) * cos(M_PI/3 * (i+1));

      double i_ = sqrt(i1*i1+i2*i2+i3*i3);

      printf("f = %f %f %f\n", f1 / f, f2 / f, f3 / f);
      printf("i = %f %f %f\n", i1 / i_, i2 / i_, i3 / i_);

      printf("--------------------\n");

    }
  return 1;
}


void test2(int argc, char *argv[])
{

  GtsSurface *s = gts_surface_new(gts_surface_class(),
				   gts_face_class(),
				   gts_edge_class(),
				   gts_vertex_class());

  double r = 1.0;
  double rs = sqrt(3) / 3 * r;
  double top = 0.4;
  for (int x = -4; x < 4; x++)
    for (int y = -4; y < 4; y++)
      {
	double cx = sqrt(3) * x;
	double cy = 2.0 * y + (x%2);

	if (cx*cx + cy*cy > 16)
	  continue;

	GtsVertex* v[7];
	GtsVertex* w[7];
	v[0] = gts_vertex_new(gts_vertex_class(), 1 * -rs + cx,  -r + cy, 0.0);
	v[1] = gts_vertex_new(gts_vertex_class(), 1 * +rs + cx,  -r + cy, 0.0);
	v[2] = gts_vertex_new(gts_vertex_class(), 2 * +rs + cx, 0.0 + cy, 0.0);
	v[3] = gts_vertex_new(gts_vertex_class(), 1 * +rs + cx,  +r + cy, 0.0);
	v[4] = gts_vertex_new(gts_vertex_class(), 1 * -rs + cx,  +r + cy, 0.0);
	v[5] = gts_vertex_new(gts_vertex_class(), 2 * -rs + cx, 0.0 + cy, 0.0);
	v[6] = gts_vertex_new(gts_vertex_class(), cx, cy, top);

	double wr = r * 0.80;
	double wrs = rs * 0.80;

	w[0] = gts_vertex_new(gts_vertex_class(), 1 * -wrs + cx, -wr + cy, top);
	w[1] = gts_vertex_new(gts_vertex_class(), 1 * +wrs + cx, -wr + cy, top);
	w[2] = gts_vertex_new(gts_vertex_class(), 2 * +wrs + cx, 0.0 + cy, top);
	w[3] = gts_vertex_new(gts_vertex_class(), 1 * +wrs + cx, +wr + cy, top);
	w[4] = gts_vertex_new(gts_vertex_class(), 1 * -wrs + cx, +wr + cy, top);
	w[5] = gts_vertex_new(gts_vertex_class(), 2 * -wrs + cx, 0.0 + cy, top);
	w[6] = gts_vertex_new(gts_vertex_class(), cx, cy, 2.0 * top);
	
	for (int i = 0; i < 6; i++)
	  {
	    int j = (i+1)%6;
	    {
	      GtsEdge *e1 = gts_edge_new(gts_edge_class(), v[i], v[j]);
	      GtsEdge *e2 = gts_edge_new(gts_edge_class(), v[j], w[i]);
	      GtsEdge *e3 = gts_edge_new(gts_edge_class(), w[i], v[i]);
	      GtsTriangle *t = gts_triangle_new(gts_triangle_class(), e1, e2, e3);
	      gts_surface_add_face(s, t);
	    }
	    {
	      GtsEdge *e1 = gts_edge_new(gts_edge_class(), v[j], w[j]);
	      GtsEdge *e2 = gts_edge_new(gts_edge_class(), w[j], w[i]);
	      GtsEdge *e3 = gts_edge_new(gts_edge_class(), w[i], v[j]);
	      GtsTriangle *t = gts_triangle_new(gts_triangle_class(), e1, e2, e3);
	      gts_surface_add_face(s, t);
	    }
	    {
	      GtsEdge *e1 = gts_edge_new(gts_edge_class(), w[6], w[i]);
	      GtsEdge *e2 = gts_edge_new(gts_edge_class(), w[i], w[j]);
	      GtsEdge *e3 = gts_edge_new(gts_edge_class(), w[j], w[6]);
	      GtsTriangle *t = gts_triangle_new(gts_triangle_class(), e1, e2, e3);
	      gts_surface_add_face(s, t);
	    }
	  }
      }
  gts_surface_write_oogl(s, stdout);
}

int main(int argc, char *argv[])
{
  test2(argc, argv);
  return 0;
}
