#include <stdio.h>

#include <cairo.h>
#include <dukglue/dukglue.h>

class grid_t {
public:
  int w, h;
  grid_t(int x, int y)
  {
    w = x;
    h = y;
  }
  int width_get()
  {
    return w;
  }
};

class CanvasRenderingContext2D {
private:
  cairo_t* ctx;
public:
  CanvasRenderingContext2D(cairo_surface_t *s)
  {
    ctx = cairo_create(s);
  }
  void beginPath()
  {
    cairo_new_path(ctx);
  }
  void closePath()
  {
    cairo_close_path(ctx);
  }
  void moveTo(double x, double y)
  {
    cairo_move_to(ctx, x, y);
  }
  void lineTo(double x, double y)
  {
    cairo_line_to(ctx, x, y);
  }
  void stroke()
  {
    cairo_stroke_preserve(ctx);
  }
  void fill()
  {
    cairo_fill_preserve(ctx);
  }
};

int main()
{
  duk_context *ctx = duk_create_heap_default();
  if (!ctx)
    {
      printf("Failed to create a Duktape heap.\n");
      exit(1);
    }

  dukglue_register_constructor<grid_t, /* constructor args: */ int, int>(ctx, "grid_t");
  dukglue_register_property(ctx, &grid_t::width_get, nullptr, "width");
  duk_push_global_object(ctx);
  duk_push_int(ctx, 10);
  duk_put_prop_string(ctx, -2, "a");

  cairo_surface_t *canvas = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 800, 800);
  CanvasRenderingContext2D *cr = new CanvasRenderingContext2D(canvas);
  dukglue_register_global(ctx, cr, "ctx");
  
  if (duk_peval_file(ctx, "dt.js") != 0)
    {
      printf("Error: %s\n", duk_safe_to_string(ctx, -1));
      duk_destroy_heap(ctx);
      exit(1);
    }
  duk_pop(ctx);  /* ignore result */

  cairo_surface_write_to_png(canvas, "canvas.png");
  
}
