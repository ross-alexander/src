#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <cairo.h>

int main()
{
  int width = 640;
  int height = 480;
  int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);

  void *data = calloc(sizeof(uint8_t), height * stride);

  assert(data);
  
  cairo_surface_t *surface = cairo_image_surface_create_for_data((void*)data, CAIRO_FORMAT_ARGB32, width, height, stride);

  assert(surface);

  cairo_t *cairo = cairo_create(surface);
  assert(cairo);

  cairo_pattern_t *p = cairo_pattern_create_linear(0, 0, width, height);

      /* offset, red, green, blue, alpha */
  cairo_pattern_add_color_stop_rgba(p, 0, 1, 0, 0, 1.0);
  cairo_pattern_add_color_stop_rgba(p, 1, 0, 1, 0, 1.0);

  cairo_set_source(cairo, p);
  //   cairo_set_source_rgb(cairo, 1.0, 0.0, 0.0);
  cairo_rectangle(cairo, 0, 0, width, height);
  cairo_fill(cairo);
  cairo_show_page(cairo);
  assert(cairo_surface_write_to_png(surface, "cairo.png") == CAIRO_STATUS_SUCCESS);
  
  cairo_destroy(cairo);
  cairo_surface_destroy(surface);
}
