#include <iostream>
#include <filesystem>

#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <fmt/printf.h>
#include "image.h"

/* ----------------------------------------------------------------------
--
-- image_t
--
---------------------------------------------------------------------- */

image_t::image_t()
{
  valid = 0;
}

image_t::image_t(fs::path p)
{
  path = p;
  valid = 0;
}

image_t::image_t(int w, int h)
{
  GeglRectangle r;
  r.x = bounds.x = 0;
  r.y = bounds.y = 0;
  r.width = bounds.width = w;
  r.height = bounds.height = h;
  valid = 1;
}

int image_t::width()
{
  return bounds.width;
}

int image_t::height()
{
  return bounds.height;
}

int image_t::is_valid()
{
  return valid;
}

std::string image_t::description()
{
  std::string s;
  if (!is_valid())
    {
      return fmt::sprintf("%s: invalid", std::string(path));
    }
  return fmt::sprintf("[%s] %s: %d x %d %s", typeid(*this).name(), std::string(path), width(), height(), is_valid() ? "valid buffer" : "no buffer");
}

int image_t::load()
{
  valid = 0;
  return 0;
}

int image_t::save()
{
  return 0;
}

int image_t::save(fs::path path)
{
  return 0;
}

int image_t::get_bounds(bounds_t &b)
{
  return 0;
}

void image_t::fill(std::string color)
{
}

image_t* image_t::scale(int)
{
  return new image_t();
}

image_t* image_t::scale(double)
{
  return new image_t();
}

void image_t::compose(image_t *i, int x, int y)
{
}

bounds_t image_t::frame(double x, double y, double w, double h, double l, const char* color)
{
  bounds_t bb;
  return bb;
}

bounds_t image_t::text(double, double, double, int, const char*, const char*)
{
  bounds_t bb;
  return bb;
}


/* ----------------------------------------------------------------------
--
-- pixbuf_t
--
---------------------------------------------------------------------- */

pixbuf_t::pixbuf_t()
{
  valid = 0;
  surface = nullptr;
  //  pixbuf = nullptr;
}

pixbuf_t::pixbuf_t(fs::path p)
{
  path = p;
  load();
}

pixbuf_t::pixbuf_t(int w, int h)
{
  //  pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 1, 8, w, h);
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  
  bounds.x = 0;
  bounds.y = 0;
  bounds.width = w;
  bounds.height = h;
  valid = 1;
}

int pixbuf_t::load()
{
  if (path.empty())
    {
      valid = 0;
      return valid;
    }
  GError *error = 0;
  GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(path.c_str(), &error);
  if (error != nullptr)
    {
      valid = 0;
      printf("** %s **\n", error->message);
      g_error_free(error);
      return valid;
    }
  valid = 1;
  bounds.x = 0;
  bounds.y = 0;
  bounds.width = gdk_pixbuf_get_width(pixbuf);
  bounds.height = gdk_pixbuf_get_height(pixbuf);

  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, bounds.width, bounds.height);
  cairo_t *cairo = cairo_create(surface);
  gdk_cairo_set_source_pixbuf(cairo, pixbuf, 0, 0);
  cairo_paint(cairo);
  cairo_destroy(cairo);
  g_object_unref(pixbuf);
  return valid;
}

int pixbuf_t::save(fs::path p)
{
  std::cout << "save " << p << "\n";
  if (!surface)
    return 0;

  char *keys[2] = { NULL, NULL };
  char *values[2] = { NULL, NULL };
  GError *error = 0;

  GdkPixbuf *pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, bounds.width, bounds.height);
  
  gdk_pixbuf_savev(pixbuf, p.c_str(), "jpeg", keys, values, &error);
  g_object_unref(pixbuf);
  return 1;
}

void pixbuf_t::fill(std::string color)
{
  GdkRGBA rgba;
  gdk_rgba_parse(&rgba, color.c_str());
  cairo_t *cairo = cairo_create(surface);
  gdk_cairo_set_source_rgba(cairo, &rgba);
  cairo_paint(cairo);
  cairo_destroy(cairo);
}

image_t* pixbuf_t::scale(double s)
{
  cairo_pattern_t *pat = cairo_pattern_create_for_surface(surface);
  cairo_matrix_t scaler;
  cairo_matrix_init_identity(&scaler);
  cairo_matrix_scale(&scaler, 1.0/s, 1.0/s);
  cairo_pattern_set_matrix(pat, &scaler);
  cairo_pattern_set_filter(pat, CAIRO_FILTER_BEST);

  double w = bounds.width * s;
  double h = bounds.height * s;
  
  cairo_surface_t *surface_new = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  cairo_t *cairo = cairo_create(surface_new);
  cairo_set_source(cairo, pat);
  cairo_paint(cairo);
  cairo_destroy(cairo);
  
  pixbuf_t *image_scaled = new pixbuf_t();
  image_scaled->surface = surface_new;
  image_scaled->valid = 1;
  image_scaled->bounds.x = 0;
  image_scaled->bounds.y = 0;
  image_scaled->bounds.width = w;
  image_scaled->bounds.height = h;

  return image_scaled;
}

image_t* pixbuf_t::scale(int size)
{
  double s;
  if (bounds.width > bounds.height)
    s = size / bounds.width;
  else
    s = size / bounds.height;

  return scale(s);
}

void pixbuf_t::compose(image_t *src, int x, int y)
{
  pixbuf_t *src_px = dynamic_cast<pixbuf_t*>(src);
  
  cairo_t *cairo = cairo_create(surface);
  cairo_set_source_surface(cairo, src_px->surface, x, y);
  cairo_paint(cairo);
  cairo_destroy(cairo);
}

bounds_t pixbuf_t::frame(double x, double y, double w, double h, double l, const char *color)
{
  GdkRGBA rgba;
  gdk_rgba_parse(&rgba, color);

  cairo_t *cairo = cairo_create(surface);
  gdk_cairo_set_source_rgba(cairo, &rgba);

  cairo_set_line_width(cairo, l);
  cairo_move_to(cairo, x, y);
  cairo_line_to(cairo, x + w, y);
  cairo_line_to(cairo, x + w, y + h);
  cairo_line_to(cairo, x, y + h);
  cairo_close_path(cairo);

  bounds_t bb;
  cairo_stroke_extents(cairo, &bb.x, &bb.y, &bb.width, &bb.height);
  cairo_stroke(cairo);
  return bb;
}

bounds_t pixbuf_t::text(double x, double y, double size, int wrap, const char *family, const char *string)
{
  cairo_t *cairo = cairo_create(surface);

  cairo_select_font_face(cairo, family, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cairo, size);

  cairo_text_extents_t extents;
  cairo_text_extents(cairo, string, &extents);

  cairo_move_to(cairo, x, y + extents.height);
  cairo_show_text(cairo, string);
  
  bounds_t bb;
  return bb;
}

/* ----------------------------------------------------------------------
--
-- gegl_t
--
---------------------------------------------------------------------- */

gegl_t::gegl_t()
{
  valid = 0;
  gbuffer = nullptr;
}

gegl_t::gegl_t(fs::path p)
{
  path = p;
  load();
}

gegl_t::gegl_t(int w, int h)
{
  GeglRectangle r;
  r.x = bounds.x = 0;
  r.y = bounds.y = 0;
  r.width = bounds.width = w;
  r.height = bounds.height = h;
  
  gbuffer = gegl_buffer_new(&r, babl_format("RGBA u8"));
  valid = 1;
}

void gegl_t::fill(std::string s)
{
  GeglNode* graph = gegl_node_new();
  /*
  GeglNode* src = gegl_node_new_child(graph,
				      "operation", "gegl:buffer-source",
				      "buffer", gbuffer,
				      nullptr);
  */
  
  GeglColor* bg = gegl_color_new(s.c_str());

  GeglNode* rectangle = gegl_node_new_child(graph,
					    "operation", "gegl:rectangle",
					    "color", bg,
					    "height", (double)bounds.height,
					    "width", (double)bounds.width,
					    "x", (double)bounds.x,
					    "y", (double)bounds.y,
					    nullptr);

  GeglNode* dst = gegl_node_new_child(graph,
				      "operation", "gegl:write-buffer",
				      "buffer", gbuffer,
				      nullptr);

  //  GeglNode *over = gegl_node_new_child(graph, "operation", "gegl:over", nullptr);
  //  gegl_node_connect_to(rectangle, "output", over, "aux");

  gegl_node_link_many(rectangle, dst, nullptr);
  gegl_node_process(dst);
  g_object_unref(bg);
}

void gegl_t::compose(image_t *i, int x, int y)
{
  gegl_t *g = dynamic_cast<gegl_t*>(i);
  
  GeglNode* graph = gegl_node_new();
  GeglNode* src = gegl_node_new_child(graph,
				      "operation", "gegl:buffer-source",
				      "buffer", g->gbuffer,
				      nullptr);
  
  GeglNode *translate = gegl_node_new_child(graph,
					   "operation", "gegl:translate",
					   "x", (double)x, "y", (double)y,
					   nullptr);
  
  GeglNode* dst = gegl_node_new_child(graph,
				      "operation", "gegl:write-buffer",
				      "buffer", gbuffer,
				      nullptr);

  //  GeglNode *over = gegl_node_new_child(graph, "operation", "gegl:over", nullptr);
  //  gegl_node_connect_to(rectangle, "output", over, "aux");

  gegl_node_link_many(src, translate, dst, nullptr);
  gegl_node_process(dst);
}



int gegl_t::save()
{
  return save(path);
}

int gegl_t::save(fs::path p)
{
  if (!gbuffer)
    return 0;

  std::cout << "save " << p << "\n";
  
  GeglNode *graph = gegl_node_new();
  GeglNode *source = gegl_node_new_child(graph,
					 "operation", "gegl:buffer-source",
					 "buffer", gbuffer,
					 nullptr);
  GeglNode *save = gegl_node_new_child(graph,
				       "operation", "gegl:save",
				       "path", p.c_str(),
				       nullptr);

  gegl_node_link_many(source, save, nullptr);
  gegl_node_process(save);
  return 1;
}

int gegl_t::load()
{
  if (path.empty())
    {
      valid = 0;
      return valid;
    }
  g_autoptr(GeglNode) graph = gegl_node_new();
  GeglNode* load = gegl_node_new_child(graph,
				       "operation", "gegl:load",
				       "path", path.c_str(),
				       nullptr);
  GeglNode* sink = gegl_node_new_child (graph,
					"operation", "gegl:buffer-sink",
					"buffer", &gbuffer,
					nullptr);


  gegl_node_link(load, sink);
  GeglRectangle r = gegl_node_get_bounding_box(sink);
  gegl_node_process(sink);
  if (gbuffer == nullptr)
    {
      valid = 0;
      return 0;
    }
  valid = 1;
  bounds.x = r.x;
  bounds.y = r.y;
  bounds.width = r.width;
  bounds.height = r.height;
  return valid;
}

image_t *gegl_t::scale(double s)
{
  g_autoptr(GeglNode) graph = gegl_node_new();

  gegl_t *scaled = new gegl_t;

  double resize_w = bounds.width * s;
  double resize_h = bounds.height * s;
  
  GeglNode* source = gegl_node_new_child(graph,
					 "operation", "gegl:buffer-source",
					 "buffer", gbuffer,
					 nullptr);
  GeglNode *scale = gegl_node_new_child(graph,
					"operation", "gegl:scale-size",
					"origin-x", 0.0,
					"origin-y", 0.0,
					"abyss-policy", GEGL_ABYSS_CLAMP,
					"sampler", GEGL_SAMPLER_NOHALO,
					"x", (double)resize_w,
					"y", (double)resize_h,
					nullptr);
  GeglNode* sink = gegl_node_new_child (graph,
					"operation", "gegl:buffer-sink",
					"buffer", &scaled->gbuffer,
					nullptr);

  gegl_node_link_many(source, scale, sink, nullptr);
  
  GeglRectangle bb = gegl_node_get_bounding_box(sink);
  scaled->valid = 1;
  scaled->bounds.x = bb.x;
  scaled->bounds.y = bb.y;
  scaled->bounds.width = bb.width;
  scaled->bounds.height = bb.height;
  
  gegl_node_process(sink);
  return scaled;
}
  

image_t* gegl_t::scale(int size)
{
  double resize_w, resize_h;
  
  if (bounds.width > bounds.height)
    {
      resize_w = size;
      resize_h = (double)bounds.height / (double)bounds.width * size;
    }
  else
    {
      resize_h = size;
      resize_w = (double)bounds.width / (double)bounds.height * size;
    }
  
  g_autoptr(GeglNode) graph = gegl_node_new();

  gegl_t *scaled = new gegl_t;

  GeglNode* source = gegl_node_new_child(graph,
					 "operation", "gegl:buffer-source",
					 "buffer", gbuffer,
					 nullptr);
  GeglNode *scale = gegl_node_new_child(graph,
					"operation", "gegl:scale-size",
					"origin-x", 0.0,
					"origin-y", 0.0,
					"abyss-policy", GEGL_ABYSS_CLAMP,
					"sampler", GEGL_SAMPLER_NOHALO,
					"x", (double)resize_w,
					"y", (double)resize_h,
					nullptr);
  GeglNode* sink = gegl_node_new_child (graph,
					"operation", "gegl:buffer-sink",
					"buffer", &scaled->gbuffer,
					nullptr);

  gegl_node_link_many(source, scale, sink, nullptr);
  
  GeglRectangle bb = gegl_node_get_bounding_box(sink);
  scaled->valid = 1;
  scaled->bounds.x = bb.x;
  scaled->bounds.y = bb.y;
  scaled->bounds.width = bb.width;
  scaled->bounds.height = bb.height;
  
  gegl_node_process(sink);
  return scaled;
}

/* ----------------------------------------------------------------------
--
-- frame
--
---------------------------------------------------------------------- */

bounds_t gegl_t::frame(double x, double y, double w, double h, double l, const char *color)
{
  
  GeglPath *path = gegl_path_new();
  GeglColor *c = gegl_color_new(color);
  
  gegl_path_append(path, 'M', x, y);
  gegl_path_append(path, 'L', x + w, y);
  gegl_path_append(path, 'L', x + w, y + h);
  gegl_path_append(path, 'L', x, y + h);
  gegl_path_append(path, 'L', x, y);
  gegl_path_append(path, 'z');

  GeglNode* graph = gegl_node_new();

  GeglNode *src = gegl_node_new_child(graph, "operation", "gegl:buffer-source",
				      "buffer", gbuffer, nullptr);
  
  GeglNode *stroke = gegl_node_new_child(graph, "operation", "gegl:path",
					 "d", path,
					 "fill-opacity", 0.5,
					 "stroke", c,
					 "stroke-width", l,
					 "stroke-hardness", 1.0,
					 nullptr);

  GeglRectangle bb = gegl_node_get_bounding_box(stroke);

  GeglNode* over = gegl_node_new_child(graph, "operation", "gegl:over", nullptr);
  gegl_node_connect(stroke, "output", over, "aux");
  
  GeglNode* dst = gegl_node_new_child(graph,
				      "operation", "gegl:write-buffer",
				      "buffer", gbuffer,
				      nullptr);

  gegl_node_link_many(src, over, dst, nullptr);
  gegl_node_process(dst);
  g_object_unref(c);
  g_object_unref(path);
  bounds_t r;
  r.x = bb.x;
  r.y = bb.y;
  r.width = bb.width;
  r.height = bb.height;
  return r;
}

/* ----------------------------------------------------------------------
--
-- text
--
---------------------------------------------------------------------- */


bounds_t gegl_t::text(double x, double y, double size, int wrap, const char *family, const char *string)
{
  GeglColor *color = gegl_color_new("#000000");
  GeglNode *graph = gegl_node_new();
  GeglNode *src = gegl_node_new_child(graph, "operation", "gegl:buffer-source",
				      "buffer", gbuffer, nullptr);
  GeglNode *text = gegl_node_new_child (graph,
					"operation", "gegl:text",
					"font", family,
					"size", size,
					"string", string,
					"wrap", wrap,
					"color", color,
					"alignment", 1,
					nullptr);
  GeglRectangle bb = gegl_node_get_bounding_box(text);
  GeglNode *translate = gegl_node_new_child(graph, "operation", "gegl:translate",
					    "x", x, "y", y,
					    nullptr);
  gegl_node_link_many(text, translate, 0);

  GeglNode *over = gegl_node_new_child(graph, "operation", "gegl:over", nullptr);
  gegl_node_connect(translate, "output", over, "aux");

  GeglNode *dst = gegl_node_new_child(graph, "operation", "gegl:write-buffer",
				      "buffer", gbuffer, nullptr);
  
  gegl_node_link_many(src, over, dst, nullptr);
  gegl_node_process(dst);
  g_object_unref(color);
  g_object_unref(graph);
  bounds_t r;
  r.x = bb.x;
  r.y = bb.y;
  r.width = bb.width;
  r.height = bb.height;
  return bounds;
}

/*
bounds_t gegl_t::get_bounds(std::string path)
{
  GeglNode *graph = gegl_node_new();
  GeglNode *load = gegl_node_new_child(graph, "operation", "gegl:load", "path", path.c_str(), nullptr);
  GeglRectangle r = gegl_node_get_bounding_box(load);
  path = std::string(path);
  valid = 1;
  g_object_unref(graph);

  bounds.x = r.x;
  bounds.y = r.y;
  bounds.width = r.width;
  bounds.height = r.height;
  return bounds;
}
*/
