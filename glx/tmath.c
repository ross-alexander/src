#include <stdio.h>

#include "mymath.h"

int main()
{
  mat4f m = myOrtho(-30, 30, -30, 30, 2, -2);
  vec4f v = {{0.0, 0.0, 0.0, 1.0}};

  printf(" ----- ortho -----\n");
  dump4m(m);

  printf(" ----- frustum -----\n");
  float s = 2.0;
  mat4f frustum = myFrustum(-s, s, -s, s, s, 2*s);
  dump4m(frustum);

  printf(" ----- lookat -----\n");

  mat4f lookat = myLookAt(0, 0, 5.5, 0, 0, 0, 0, 1, 0);
  dump4m(lookat);

  printf(" ----- proj = frustum*lookat -----\n");

  mat4f proj = mul4mf(frustum, lookat);
  dump4m(proj);

  printf("\n");

  for(int i = -5; i <= 5; i += 1)
    {
      v.v[2] = (float)i;
      printf("  Orig:"); dump4v(v);
      vec4f r = mul4vf(proj, v);
      printf("  Mult:"); dump4v(r);
      for (int j = 0; j < 4; j++)
	r.v[j] = r.v[j] / r.v[3];

      if (r.v[2] >= -1.0 && r.v[2] <= 1.0)
	printf("* ");
      else
	printf("  ");
      printf("Scal:"); dump4v(r);
      printf("\n");
    }
}
