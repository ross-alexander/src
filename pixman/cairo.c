/* ----------------------------------------------------------------------
 *
 * 2025-10-07: Identical attempt to do GDK-Pixbuf to Cairo without needing GDK
 *
 ---------------------------------------------------------------------- */

#include <assert.h>

#include <cairo.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <pixman.h>
#include <babl/babl.h>

const Babl *gimp_pixbuf_get_format (GdkPixbuf *pixbuf)
{
  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);
  switch (gdk_pixbuf_get_n_channels (pixbuf))
    {
    case 3: return babl_format ("R'G'B' u8");
    case 4: return babl_format ("R'G'B'A u8");
    }
  g_return_val_if_reached (NULL);
}

pixman_image_t *pixman_image_from_file (const char *filename, pixman_format_code_t format)
{
    GdkPixbuf *pixbuf;
    pixman_image_t *image;
    int width, height;
    uint32_t *data, *d;
    uint8_t *gdk_data;
    int n_channels;
    int j, i;
    int stride;

    if (!(pixbuf = gdk_pixbuf_new_from_file (filename, NULL)))
	return NULL;

    image = NULL;

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);
    n_channels = gdk_pixbuf_get_n_channels (pixbuf);
    gdk_data = gdk_pixbuf_get_pixels (pixbuf);
    stride = gdk_pixbuf_get_rowstride (pixbuf);

    if (!(data = malloc (width * height * sizeof (uint32_t))))
	goto out;

    d = data;
    for (j = 0; j < height; ++j)
    {
	uint8_t *gdk_line = gdk_data;

	for (i = 0; i < width; ++i)
	{
	    int r, g, b, a;
	    uint32_t pixel;

	    r = gdk_line[0];
	    g = gdk_line[1];
	    b = gdk_line[2];

	    if (n_channels == 4)
		a = gdk_line[3];
	    else
		a = 0xff;

	    r = (r * a + 127) / 255;
	    g = (g * a + 127) / 255;
	    b = (b * a + 127) / 255;

	    pixel = (a << 24) | (r << 16) | (g << 8) | b;

	    *d++ = pixel;
	    gdk_line += n_channels;
	}
	gdk_data += stride;
    }

    image = pixman_image_create_bits(format, width, height, data, width * 4);

out:
    g_object_unref (pixbuf);
    return image;
}

void cvt_pixman(const char *ipath, const char *opath)
{
  pixman_image_t *pimage = pixman_image_from_file(ipath, PIXMAN_a8r8g8b8);

  int width = pixman_image_get_width (pimage);
  int height = pixman_image_get_height (pimage);
  int stride = pixman_image_get_stride (pimage);
  cairo_surface_t *cimage;
  cairo_format_t format;

  if (pixman_image_get_format (pimage) == PIXMAN_x8r8g8b8)
    format = CAIRO_FORMAT_RGB24;
  else
    format = CAIRO_FORMAT_ARGB32;

  cimage = cairo_image_surface_create_for_data((uint8_t *)pixman_image_get_data (pimage),
					       format, width, height, stride);

  cairo_surface_write_to_png(cimage, opath);
}

int cvt_babl(const char *ipath, const char *opath)
{
  GdkPixbuf *px;
  cairo_surface_t *cimage;
  cairo_format_t format;

  if (!(px = gdk_pixbuf_new_from_file (ipath, NULL)))
    return 1;

  uint32_t width = gdk_pixbuf_get_width(px);
  uint32_t height = gdk_pixbuf_get_height(px);
  uint32_t alpha = gdk_pixbuf_get_has_alpha(px);
  uint32_t n_channels = gdk_pixbuf_get_n_channels(px);
  uint32_t depth = gdk_pixbuf_get_bits_per_sample(px);
  uint32_t stride = gdk_pixbuf_get_rowstride(px);

  babl_init();

  // GdkColorspace color = gdk_pixbuf_get_colorspace(px);

  /* Report image details */
  
  printf("%d × %d × %d [%d : %d] (%s) %s\n", width, height, depth, stride, n_channels, alpha ? "alpha" : "no alpha", ipath);

  const Babl *dst;
  const Babl *src;

  switch (n_channels)
    {
    case 3: src = babl_format ("R'G'B' u8");
    case 4: src = babl_format ("R'G'B'A u8");
    }
  if (alpha)
    src = babl_format_new(babl_model("RGBA"), babl_type("u8"),
				 babl_component("B"),
				 babl_component("G"),
				 babl_component("R"),
				 babl_component("A"),
				 NULL);
  else
    src = babl_format_new(babl_model("R'G'B'"), babl_type("u8"),
				 babl_component("R'"),
				 babl_component("G'"),
				 babl_component("B'"),
				 NULL);
  if (alpha)
    {
      dst = babl_format("cairo-ARGB32");
      format = CAIRO_FORMAT_ARGB32;
    }
  else
    {
      dst = babl_format("cairo-RGB24");
      format = CAIRO_FORMAT_RGB24;
    }

  printf("Output to cairo [%d : %d]\n", format, cairo_format_stride_for_width(format, width) / width);
  
  cimage = cairo_image_surface_create (format, width, height);
  uint32_t c_stride = cairo_image_surface_get_stride(cimage);
  
  babl_process_rows(babl_fish(src, dst),
		    gdk_pixbuf_get_pixels(px), stride,
		    cairo_image_surface_get_data(cimage), c_stride,
		    width,
		    height);
  cairo_surface_write_to_png(cimage, opath);
  return 0;
}

/* ----------------------------------------------------------------------
 *
 * main
 *
 ---------------------------------------------------------------------- */
int main()
{
  cvt_pixman("input.png", "pixman.png");
  cvt_babl("input.png", "babl.png");
}
