/* ----------------------------------------------------------------------
--
-- scale.cc
--
-- 2021-08-17: Ross Alexander
--    Update gegl to reflect change to pixbuf-save.
--
---------------------------------------------------------------------- */

#include <map>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <cairo.h>
#include <gegl.h>

/* ----------------------------------------------------------------------
--
-- structs
--
---------------------------------------------------------------------- */

struct scale_t {
  GdkPixbuf *src_pixbuf;
  GdkPixbuf *dst_pixbuf;
  int src_width;
  int src_height;
  int src_depth;
  int src_alpha;
  int dst_width;
  int dst_height;
  double scale;
};

typedef int (*scalef)(scale_t*);

/* ----------------------------------------------------------------------
--
-- format_filename
--
---------------------------------------------------------------------- */

char *format_filename(const char *path, const char *format, const char* suffix)
{
  int i;
  int j = strlen(format) + strlen(suffix) + 2;
  unsigned int l = strlen(path);
  for (i = l-1; i; i--)
    {
      if (path[i] == '.') break;
    }
  if (i == 0) i = l;

  char *s = (char*)calloc(1, i + j + 3);
  strncat(s, path, i);
  strcat(s, ".");
  strcat(s, format);
  strcat(s, ".");
  strcat(s, suffix);
  return s;
}

/* ----------------------------------------------------------------------
--
-- scale_pixbuf
--
---------------------------------------------------------------------- */

int scale_pixbuf(scale_t *s)
{
  GdkColorspace color = gdk_pixbuf_get_colorspace(s->src_pixbuf);
  s->dst_pixbuf = gdk_pixbuf_new(color, s->src_alpha, s->src_depth, s->dst_width, s->dst_height);
  gdk_pixbuf_fill(s->dst_pixbuf, 0xffffffff);
  gdk_pixbuf_scale(s->src_pixbuf, s->dst_pixbuf, 0, 0, s->dst_width, s->dst_height, 0.0, 0.0, s->scale, s->scale, GDK_INTERP_HYPER);
  return 1;
}

/* ----------------------------------------------------------------------
--
-- scale_cairo
--
----------------------------------------------------------------------*/

int scale_cairo(scale_t *s)
{
  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, s->src_width, s->src_height);
  cairo_t *cr = cairo_create(surface);
  gdk_cairo_set_source_pixbuf(cr, s->src_pixbuf, 0, 0);
  cairo_paint(cr);
  cairo_destroy(cr);
      
  cairo_pattern_t *pat = cairo_pattern_create_for_surface(surface);
  cairo_matrix_t scaler;
  cairo_matrix_init_identity(&scaler);
  cairo_matrix_scale(&scaler, 1.0/s->scale, 1.0/s->scale);
  cairo_pattern_set_matrix(pat, &scaler);
  cairo_pattern_set_filter(pat, CAIRO_FILTER_BEST);
  
  cairo_surface_t *surface_new = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, s->dst_width, s->dst_height);
  cr = cairo_create(surface_new);
  cairo_set_source(cr, pat);
  cairo_paint(cr);

  s->dst_pixbuf = gdk_pixbuf_get_from_surface(surface_new, 0, 0, s->dst_width, s->dst_height);
  cairo_surface_destroy(surface_new);
  cairo_surface_destroy(surface);
  return 1;
}

/* ----------------------------------------------------------------------
--
-- scale_gegl
--
---------------------------------------------------------------------- */

int scale_gegl(scale_t *s)
{

  g_autoptr (GeglNode) graph = NULL;

  graph = gegl_node_new();

  /* --------------------
     Get data from pixbuf
     -------------------- */
  
  GeglNode *load = gegl_node_new_child(graph,
				       "operation", "gegl:pixbuf",
				       "pixbuf", s->src_pixbuf,
				       NULL);

  /* --------------------
     scale buffer
     --------------------*/
  
  GeglNode *scale = gegl_node_new_child(graph,
					"operation", "gegl:scale-size",
					"origin-x", 0.0,
					"origin-y", 0.0,
					"abyss-policy", GEGL_ABYSS_NONE,
					"sampler", GEGL_SAMPLER_CUBIC,
					"x", (double)s->dst_width,
					"y", (double)s->dst_height,
					NULL);
  /* --------------------
     save buffer to new pixbuf
     -------------------- */
  
  GeglNode *save = gegl_node_new_child(graph,
				       "operation", "gegl:save-pixbuf",
				       NULL);

  /* --------------------
     Link and process
     -------------------- */

  gegl_node_link_many(load, scale, save, NULL);
  gegl_node_process(save);

  /* --------------------
     Get pixbuf pointer
     -------------------- */
  
  gegl_node_get(save, "pixbuf", &s->dst_pixbuf, NULL);

  return 1;
}

/* ----------------------------------------------------------------------
--
-- scale
--
---------------------------------------------------------------------- */

int scale(char *file, int size, double ratio, std::string alg, std::string format)
{
  GError *error = NULL;

  scale_t s;

  std::map<std::string, scalef> fmap;
  fmap.emplace("pixbuf", &scale_pixbuf);
  fmap.emplace("cairo", &scale_cairo);
  fmap.emplace("gegl", &scale_gegl);
  
  s.src_pixbuf = gdk_pixbuf_new_from_file(file, &error);
  assert(s.src_pixbuf != NULL);

  s.src_width = gdk_pixbuf_get_width(s.src_pixbuf);
  s.src_height = gdk_pixbuf_get_height(s.src_pixbuf);
  s.src_alpha = gdk_pixbuf_get_has_alpha(s.src_pixbuf);
  s.src_depth = gdk_pixbuf_get_bits_per_sample(s.src_pixbuf);

  /* --------------------
     scale image to longest side = size
     -------------------- */

  if (size > 0)
    {
      ratio = (double)s.src_width / s.src_height;
      if (ratio > 1.0)
	{
	  s.dst_width = size;
	  s.dst_height = size / ratio;
	  s.scale = (double)size / s.src_width;
	}
      else
	{
	  s.dst_width = size * ratio;
	  s.dst_height = size;
	  s.scale = (double)size / s.src_height;
	}
    }
  else
    {
      s.dst_width = s.src_width * ratio;
      s.dst_height = s.src_height * ratio;
      s.scale = ratio;
    }
  
  // Check if algorithm exists
  
  if (!fmap.count(alg))
    {
      fprintf(stderr, "algorithm %s not found\n", alg.c_str());
      exit(1);
    }

  // Get scaling algorithm and call it
  
  scalef f = fmap.at(alg);
  int res = (*f)(&s);
  if (res && s.dst_pixbuf)
    {
      char *path = format_filename(file, alg.c_str(), format.c_str());
      printf("%s: %d x %d -> %d x %d using %s -> %s\n", file, s.src_width, s.src_height, s.dst_width, s.dst_height, alg.c_str(), path);
      
      char *keys[2] = { NULL, NULL };
      char *values[2] = { NULL, NULL };
      
      if (format.compare("png") == 0)
	{
	  keys[0] = (char*)"compression";
	  values[0] = (char*)"9";
	}
      if (format.compare("jpeg") == 0)
	{
	  keys[0] = (char*)"quality";
	  values[0] = (char*)"90";
	}
      gdk_pixbuf_savev(s.dst_pixbuf, path, format.c_str(), keys, values, &error);
      
      if (error)
	printf("%s: %s\n", path, error->message);
      free(path);
    }
  return 1;
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  gegl_init(&argc, &argv);

  int ch;
  int size = 0;
  double ratio = 1.0;

  std::string alg = "pixbuf";
  std::string format = "png";
  
  while ((ch = getopt(argc, argv, "a:r:s:f:")) != EOF)
    switch(ch)
      {
      case 's':
	size = strtol(optarg, NULL, 10);
	break;
      case 'r':
	ratio = strtod(optarg, NULL);
	break;
      case 'a':
	alg = optarg;
	break;
      case 'f':
	format = optarg;
	break;
      }

  if ((argc - optind) < 1)
    {
      fprintf(stderr, "%s: [filename]\n", argv[0]);
      exit(1);
    }
  for (int i = optind; i < argc; i++)
    scale(argv[i], size, ratio, alg, format);

  gegl_exit();
  return 0;
}
