#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <glib.h>
#include <math.h>

#include "mandel.h"

rgb3d convert_hsv_to_rgb(double hue, double s, double v)
{
  double nr, ng, nb;          /* rgb values of 0.0 - 1.0      */
  double p1, p2, p3, i, f;
  double xh;
  //        extern float hue,  s,  v;  /* hue (0.0 to 360.0, is circular, 0=360)
	//                                      s and v are from 0.0 - 1.0) */
  if (hue == 360.0)
    hue = 0.0;           /* (THIS LOOKS BACKWARDS)       */

  xh = hue / 60.0;                   /* convert hue to be in 0,6       */
  i = floor((double)xh);            /* i = greatest integer <= h    */
  f = xh - i;                       /* f = fractional part of h     */
  p1 = v * (1 - s);
  p2 = v * (1 - (s * f));
  p3 = v * (1 - (s * (1 - f)));
  
  switch ((int)i)
    {
    case 0:
      nr = v;
      ng = p3;
      nb = p1;
      break;
    case 1:
      nr = p2;
      ng = v;
      nb = p1;
      break;
    case 2:
      nr = p1;
      ng = v;
      nb = p3;
      break;
    case 3:
      nr = p1;
      ng = p2;
      nb = v;
      break;
    case 4:
      nr = p3;
      ng = p1;
      nb = v;
      break;
    case 5:
      nr = v;
      ng = p1;
      nb = p2;
      break;
    }
  rgb3d res;
  res.red = nr;
  res.green = ng;
  res.blue = nb;
  return res;
}


static int mandel_pixel(int iters, double a, double b)
{	
  register int count;
  double t, tx, ty, tx2, ty2;

  count = 0;
  tx = a;
  ty = b;
  
  /* general Mandelbrot algorithm ... */
  while ((((tx2 = tx*tx) + (ty2 = ty*ty)) < 4.0) && (count <= iters))
    {
      t = tx2 - ty2 + a;
      ty = 2*tx*ty + b;
      tx = t;
      count++;
    }
  //  printf("%f %f %d\n", a, b, count);
  return (count > iters) ? 0 : count;
}

mandel_image* mandel_image_create(int colors, double xmin, double xmax, double ymin, double ymax, int width, int height)
{
  int depth;

  assert(colors > 0 && colors <= 256);

  if (colors > 65536)
    depth = 4;
  else if (colors > 256)
    depth = 2;
  else
    depth = 1;

  unsigned char* map = (unsigned char*)malloc(width * height * depth);

  mandel_image* image = calloc(sizeof(mandel_image), 1);
  image->width = width;
  image->height = height;
  image->depth = depth * 8;
  image->palette_size = colors;
  image->data = map;

  int c, xplot, yplot;
  double x, y;
  double xscale = (xmax - xmin) / width;
  double yscale = (ymax - ymin) / height;

  printf("Creating mandelbrot image (%d, %d) with %d colours.\n", width, height, colors);

  colors--;
  y = ymin;
  yplot = 0;
  for (yplot = 0; yplot < height; yplot++)
    {
      x = xmin;
      xplot = 0;
      for (xplot = 0; xplot < width; xplot++)
	{
	  c = mandel_pixel(colors, x, y);
	  map[(height - yplot) * width + xplot] = c;
	  x += xscale;
	}
      y += yscale;
    }
  return image;
}

mandel_image *mandel_image_create_default(int width, int height, int depth)
{
  return mandel_image_create(256, -1.0, 1.0, -1.0, 1.0, width, height);
}

void mandel_image_create_hsv_palette(mandel_image *image)
{
  int colors = image->palette_size;
  rgb3i* palette = (rgb3i*)malloc(colors * sizeof(rgb3i));
  for (int i = 0; i < colors; i++)
    {
      double hue = fmod(240 - 360.0 / colors * i * 4.0, 360.0);
      rgb3d c = convert_hsv_to_rgb(hue, 0.8, 0.8);
      palette[i].red = c.red * 255;
      palette[i].green = c.green * 256;
      palette[i].blue = c.blue * 256;
    }

  palette[0].red = palette[0].green = palette[0].blue = 0;
  image->palette = palette;
}

typedef guint8 rgb[3];

void mandel_image_8to24(mandel_image *image)
{
  rgb* buf = malloc(image->width * image->height * sizeof(rgb));

  for (int i = 0; i < image->height; i++)
    for (int j = 0; j < image->width; j++)
      {
	guint8 v = image->data[i * image->width + j];
	assert(v >= 0 && v < image->palette_size);

	buf[i * image->width + j][0] = image->palette[v].red;
	buf[i * image->width + j][1] = image->palette[v].green;
	buf[i * image->width + j][2] = image->palette[v].blue;
      }
  free(image->data);
  image->data = (unsigned char*)buf;
  free(image->palette);
  image->palette_size = 0;
}


void mandel_image_destroy(mandel_image *image)
{
  if (image->data)
    free(image->data);
  free(image);
}
