/* ----------------------------------------------------------------------
--
-- libpng.c
--
-- 2019-08-29: Ross Alexander
--   Update comments
--
---------------------------------------------------------------------- */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <glib.h>

#include <cairo.h>
#include <babl/babl.h>

#ifdef HAVE_PNG_H
#include <zlib.h>
#include <png.h>
#endif

#include "mandel.h"

/* ----------------------------------------------------------------------
--
-- mandel_write_png
--
---------------------------------------------------------------------- */

void mandel_write_png(mandel_image *image, char *filename)
{
  int i;
  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr;
  png_colorp pal_ptr = NULL;
  int compression = 0;

  fp = fopen(filename, "wb");
  if (!fp)
    return;

  if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
    {
      fclose(fp);
      return;
    }

  if ((info_ptr = png_create_info_struct(png_ptr)) == NULL)
    {
      fclose(fp);
      png_destroy_write_struct(&png_ptr,  0);
      return;
    }

  png_init_io(png_ptr, fp);

  png_set_IHDR(png_ptr, info_ptr, image->width, image->height,
	       image->depth,
	       image->palette_size > 0 ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_RGB,
	       PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE,
	       PNG_FILTER_TYPE_BASE);

  if (image->palette_size > 0)
    {
      pal_ptr = (png_colorp)calloc(sizeof(png_color), image->palette_size);
      for (i = 0; i < image->palette_size; i++)
	{
	  pal_ptr[i].red = image->palette[i].red;
	  pal_ptr[i].green = image->palette[i].green;
	  pal_ptr[i].blue = image->palette[i].blue;
	}
      png_set_PLTE(png_ptr, info_ptr, pal_ptr, image->palette_size);
    }

  png_write_info(png_ptr, info_ptr);

  printf("Writing image (%d x %d) ...\n", image->width, image->height);

  int stride = image->palette_size > 0 ? image->width * (image->depth >> 3) : image->width * (image->depth >> 3) * 3;

  for (i = 0; i < image->height; i++)
    {
      png_bytep row_pointers = (png_bytep)&(image->data[i * stride]);
      png_write_rows(png_ptr, &row_pointers, 1);
    }
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(fp);
}

/* ----------------------------------------------------------------------
--
-- mandel_write_cm
--
---------------------------------------------------------------------- */

int mandel_write_cm(mandel_image *image, char *filename)
{
  FILE *stream;
  if ((stream = fopen(filename, "w")) == NULL)
    {
      fprintf(stderr, "Cannot open file mandel.cm for writing.\n");
      return 0;
    }
  fwrite(&image->width, sizeof(image->width), 1, stream);
  fwrite(&image->height, sizeof(image->height), 1, stream);
  fwrite(&image->palette_size, sizeof(image->palette_size), 1, stream);
  fwrite(image->palette, sizeof(rgb3i), image->palette_size, stream);
  fwrite(image->data, sizeof(guint8), image->width * image->height, stream);
  fclose(stream);
  return 1;
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  int width = 1500;
  int height = 1000;
  FILE *stream;
  int colors = 256;
  int i;

  int write_cm = 0;
  int write_babl = 1;
  int write_png = 1;
  int write_cairo = 1;

  int ch;
  while ((ch = getopt(argc, argv, "w:h:bcp")) != EOF)
    {
      switch(ch)
	{
	case 'w':
	  width = strtol(optarg, 0, 10);
	  break;
	case 'h':
	  height = strtol(optarg, 0, 10);
	  break;
	}
    }
  
  mandel_image* image = mandel_image_create(256, -2.0, 1.0, -1.0, 1.0, width, height);
  mandel_image_create_hsv_palette(image);
  
  if (write_cm)
    mandel_write_cm(image, "mandel.cm");

  if (write_png)
    mandel_write_png(image, "mandel-index.png");

  mandel_image_8to24(image);
  if (write_png)
    mandel_write_png(image, "mandel-truecolor.png");
  
  typedef guint8 rgb[3];
  typedef guint8 rgba[4];

  /* --------------------
     Use cairo to save PNG
     -------------------- */
  
  if (write_cairo)
    {
      cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
      rgba *buf = (rgba*)cairo_image_surface_get_data(surface);

      for (int i = 0; i < height; i++)
	for (int j = 0; j < width; j++)
	  {
	    buf[i * width + j][0] = image->data[(i * width + j) * 3 + 2];
	    buf[i * width + j][1] = image->data[(i * width + j) * 3 + 1];
	    buf[i * width + j][2] = image->data[(i * width + j) * 3 + 0];
	    buf[i * width + j][3] = 0;
	  }
      cairo_surface_write_to_png(surface, "mandel-cairo.png");
      cairo_surface_destroy(surface);
    }

  /* --------------------
     Use babl to create cairo surface
     -------------------- */
  
  if (write_babl)
    {
      cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
      babl_init();
      const Babl *src            = babl_format ("RGB u8");
      const Babl *dst = babl_format_new (babl_model ("RGB"),
					 babl_type ("u8"),
					 babl_component ("B"),
					 babl_component ("G"),
					 babl_component ("R"),
					 babl_component ("PAD"),
					 NULL);
      
      babl_process(babl_fish(src, dst), image->data, cairo_image_surface_get_data(surface), width * height);
      cairo_surface_write_to_png(surface, "mandel-babl.png");
      cairo_surface_destroy(surface);
    }
  mandel_image_destroy(image);
}
