#include <gegl.h>
#include <fmt/printf.h>

int scale(const char *path)
{
  GeglRectangle size;

  for (int i = 5; i < 100; i+= 5)
    {
      g_autoptr (GeglNode)  graph = gegl_node_new();
        
      GeglNode* load = gegl_node_new_child(graph,
					   "operation", "gegl:load",
					   "path", path,
					   0);
      
      size = gegl_node_get_bounding_box (load);
      
      struct scale_t {
	double dst_width;
	double dst_height;
      };
      
      scale_t *s = new scale_t;
      s->dst_width = size.width * (i / 100.0);
      s->dst_height = size.height * (i / 100.0);
      
      GeglNode *scale = gegl_node_new_child(graph,
					    "operation", "gegl:scale-size",
					    "origin-x", 0.0,
					    "origin-y", 0.0,
					    "abyss-policy", GEGL_ABYSS_CLAMP,
					    "sampler", GEGL_SAMPLER_NOHALO,
					    "x", (double)s->dst_width,
					    "y", (double)s->dst_height,
					    NULL);

      std::string savepath = fmt::sprintf("gegl-%02d.jpg", i);
      
      GeglNode *save = gegl_node_new_child(graph,
					   "operation", "gegl:save",
					   "path", savepath.c_str(),
					   NULL);

      gegl_node_link_many(load, scale, save, NULL);
      gegl_node_process(save);
    }
  return 1;
}

int main(int argc, char *argv[])
{
  gegl_init(&argc, &argv);

  for (int j = 1; j < argc; j++)
    {
      scale(argv[j]);
    }
  return 0;
}
