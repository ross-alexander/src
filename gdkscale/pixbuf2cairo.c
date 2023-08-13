/* ----------------------------------------------------------------------
   --
   -- pixbuf2cairo
   --
   -- 2023-08-12: Use babl to convert gdk-pixbuf to cairo
   --
   --
   ---------------------------------------------------------------------- */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <getopt.h>

#include <babl/babl.h>
#include <cairo/cairo.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


/* ----------------------------------------------------------------------
   --
   -- main
   --
   ---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  int keep_alpha = 1;
  int ch;

  /* allow alpha to removed */
  
  while ((ch = getopt(argc, argv, "a")) != EOF)
    switch(ch)
      {
      case 'a':
	keep_alpha = 1 - keep_alpha;
	break;
      }
  
  if ((argc - optind) < 1)
    {
      fprintf(stderr, "%s: <path to image>\n", argv[0]);
      exit(1);
    }
  
  const char *path = argv[optind];
  GError *error = 0;

  babl_init();

  /* Load image using gdk-pixbuf */
  
  GdkPixbuf *px = gdk_pixbuf_new_from_file(path, &error);
  if (px == 0)
    {
      fprintf(stderr, "%s: Failed to load image %s\n", argv[0], path);
      exit(1);
    }

  /* Get image particulars */

  uint32_t width = gdk_pixbuf_get_width(px);
  uint32_t height = gdk_pixbuf_get_height(px);
  uint32_t alpha = gdk_pixbuf_get_has_alpha(px);
  uint32_t depth = gdk_pixbuf_get_bits_per_sample(px);
  GdkColorspace color = gdk_pixbuf_get_colorspace(px);

  /* Report image details */
  
  printf("%d × %d × × %d (%s) %s\n", width, height, depth, alpha ? "alpha" : "no alpha", path);

  /* Select formats */
  
  const Babl *src_format, *dst_format;
  cairo_format_t cairo_format;
  
  if (alpha)
    src_format = babl_format_new(babl_model("RGBA"), babl_type("u8"),
				 babl_component("B"),
				 babl_component("G"),
				 babl_component("R"),
				 babl_component("A"),
				 0);
  else
    src_format = babl_format_new(babl_model("R'G'B'"), babl_type("u8"),
				 babl_component("R'"),
				 babl_component("G'"),
				 babl_component("B'"),
				 0);

  /* Check if alpha is in destination */
  
  if (alpha & keep_alpha)
    {
      cairo_format = CAIRO_FORMAT_ARGB32;
      dst_format = babl_format("cairo-ARGB32");      
    }
  else
    {
      cairo_format = CAIRO_FORMAT_RGB24;
      dst_format = babl_format("cairo-RGB24");      
    }

  /* Create destination surface */
  
  cairo_surface_t *surface = cairo_image_surface_create (cairo_format, width, height);
  
  babl_process (babl_fish(src_format, dst_format),
		gdk_pixbuf_get_pixels(px),
		cairo_image_surface_get_data(surface),
		width * height);

  /* Save to PNG */
  
  cairo_surface_write_to_png(surface, "babl.png");
  return 0;
}
