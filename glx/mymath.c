#include <stdio.h>
#include <math.h>

#include "mymath.h"

mat4f myFrustum(float l, float r, float b, float t, float n, float f)
{
  mat4f m;

   m.m[ 0] = 2.0f * n / (r-l);
   m.m[ 1] = 0.0f;
   m.m[ 2] = 0.0f;
   m.m[ 3] = 0.0f;

   m.m[ 4] = 0.0f;
   m.m[ 5] = 2.0f * n / (t-b);
   m.m[ 6] = 0.0f;
   m.m[ 7] = 0.0f;

   m.m[ 8] = (r+l)/(r-l);
   m.m[ 9] = (t+b)/(t-b);
   m.m[10] = -(f+n) / (f-n);
   m.m[11] = -1.0f;

   m.m[12] = 0.0;
   m.m[13] = 0.0;
   m.m[14] = -2.0 * (f*n) / (f-n);
   m.m[15] = 0.0f;
   return m;
}

mat4f myOrtho(float l, float r, float b, float t, float n, float f)
{
  /* ortho matrix */
  mat4f m;
  m.m[ 0] = 2.0f / (r - l);
  m.m[ 1] = 0.0f;
  m.m[ 2] = 0.0f;
  m.m[ 3] = 0.0f;
  
  m.m[ 4] = 0.0f;
  m.m[ 5] = 2.0f / (t - b);
  m.m[ 6] = 0.0f;
  m.m[ 7] = 0.0f;
  
  m.m[ 8] = 0.0f;
  m.m[ 9] = 0.0f;
  m.m[10] = -2.0f / (f - n);
  m.m[11] = 0.0f;
  
  m.m[12] = -(r + l) / (r - l);
  m.m[13] = -(t + b) / (t - b);
  m.m[14] = -(f + n) / (f - n);
  m.m[15] = 1.0f;
  return m;
}

void dump4m(mat4f m)
{
  for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
	printf("%6.2f ", (double)m.m[i + 4*j]);
      printf("\n");
    }
}

void dump4v(vec4f v)
{
  for (int j = 0; j < 4; j++)
    printf("%6.2f ", (double)v.v[j]);
  printf("\n");
}

vec4f sub4vf(vec4f a, vec4f b)
{
  vec4f c;
  for (int i = 0; i < 4; i++)
    c.v[i] = a.v[i] - b.v[i];
  return c;
}

float dot4vf(vec4f a, vec4f b)
{
  float r = 0.0;
  for (int i = 0; i < 4; i++)
    r += a.v[i] * b.v[i];
  return r;
}

vec4f mul4vf(mat4f m, vec4f v)
{
  vec4f r;
  for (int i = 0; i < 4; i++)
    {
      float s = 0.0;
      for (int j = 0; j < 4; j++)
	{
	  //	  printf("%4.2f x %4.2f = %4.2f\n", m.m[4*j+i], v.v[j], m.m[4*j+i] * v.v[j]);
	  s += m.m[4*j+i] * v.v[j];
	}
      //      printf("\n");
      r.v[i] = s;
    }
  return r;
}

vec4f norm4vf(vec4f a)
{
  vec4f r;
  float b = sqrt(dot4vf(a, a));
  for (int i = 0; i < 4; i++)
    r.v[i] = a.v[i] / b;
  return r;
}

vec4f cross4vf(vec4f a, vec4f b)
{
  vec4f c;
  
  c.v[0] = a.v[1] * b.v[2] - a.v[2] * b.v[1];
  c.v[1] = a.v[2] * b.v[0] - a.v[0] * b.v[2];
  c.v[2] = a.v[0] * b.v[1] - a.v[1] * b.v[0];
  c.v[3] = 0.0;
  return c;
}

mat4f myLookAt(float eye_x, float eye_y, float eye_z, float at_x, float at_y, float at_z, float up_x, float up_y, float up_z)
{
   vec4f at = {{at_x,   at_y,   at_z, 0.0}};
   vec4f eye = {{eye_x, eye_y, eye_z, 0.0}};
   vec4f up = {{up_x,   up_y,   up_z, 0.0}};

   //   printf("At:   "); dump4v(at);
   //   printf("Eye:  "); dump4v(eye);

   vec4f zaxis = norm4vf(sub4vf(eye, at));
   vec4f xaxis = norm4vf(cross4vf(up, zaxis));
   vec4f yaxis = cross4vf(zaxis, xaxis);

   //   printf("Zaxis: "); dump4v(zaxis);

   // zaxis = normal(At - Eye)
   // xaxis = normal(cross(Up, zaxis))
   // yaxis = cross(zaxis, xaxis)

   // xaxis.x           yaxis.x           zaxis.x          0
   // xaxis.y           yaxis.y           zaxis.y          0
   // xaxis.z           yaxis.z           zaxis.z          0
   // -dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye)  l


   mat4f m = {{
       xaxis.v[0],
       yaxis.v[0],
       zaxis.v[0],
       0.0,

       xaxis.v[1],
       yaxis.v[1],
       zaxis.v[1],
       0.0,

       xaxis.v[2],
       yaxis.v[2],
       zaxis.v[2],
       0.0,

       -dot4vf(xaxis, eye),
       -dot4vf(yaxis, eye),
       -dot4vf(zaxis, eye),
       1.0
     }};
   return m;
}

mat4f ident4mf(void)
{
  mat4f m;
  for (int i = 0; i < 16; i++) m.m[i] = 0.0d;
  m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1.0d;
  return m;
}

mat4f mul4mf(mat4f m, mat4f n)
{
  mat4f o;
  for (int row = 0; row < 4; row++)
    for (int col = 0; col < 4; col++)
      {
	float r = 0.0;
	for (int k = 0; k < 4; k++)
	  {
	    r += m.m[4 * k + row] * n.m[4 * col + k];
	  }
	o.m[4 * col + row] = r;
      }
  return o;
}

mat4f rotatez(float phi)
{
  mat4f n = ident4mf();
  /* --------------------
     rotate around z axis
     -------------------- */

  n.m[0] = cos(phi);
  n.m[1] = sin(phi);
  n.m[4] = -sin(phi);
  n.m[5] = cos(phi);
  return n;
}

mat4f rotatey(float phi)
{
  mat4f n = ident4mf();

  /* --------------------
     rotate around y axis
     -------------------- */

  n.m[0] = cos(phi);
  n.m[2] = sin(phi);
  n.m[8] = -sin(phi);
  n.m[10] = cos(phi);

  return n;
}

#ifdef X
void translate(float m[], float x, float y, float z)
{
  float n[16], o[16];
  identity(n);
  n[12] = x;
  n[13] = y;
  n[14] = z;
  mmult(o, m, n);
  mcopy(m, o);
}
#endif

