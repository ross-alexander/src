/* ----------------------------------------------------------------------
--
-- path-example
--
-- 2020-01-04: Ross Alexander
--
---------------------------------------------------------------------- */

#include <boost/program_options.hpp>
#include <stdio.h>
#include <gegl.h>

/* ----------------------------------------------------------------------
--
-- pe_load
--
---------------------------------------------------------------------- */

GeglBuffer* pe_load(std::string path)
{
  GeglBuffer *buffer;
  GeglNode *graph = gegl_node_new();

  GeglNode *load = gegl_node_new_child(graph, "operation", "gegl:load",
				       "path", path.c_str(), nullptr);

  GeglNode *sink = gegl_node_new_child(graph, "operation", "gegl:buffer-sink",
					 "buffer", &buffer, nullptr);
  
  gegl_node_link_many(load, sink, nullptr);
  gegl_node_process(sink);
  g_object_unref(graph);
  
  return buffer;
}

/* ----------------------------------------------------------------------
--
-- pe_save
--
---------------------------------------------------------------------- */

void pe_save(std::string path, GeglBuffer *buffer)
{
  GeglNode *graph = gegl_node_new();
  GeglNode *src = gegl_node_new_child(graph, "operation", "gegl:buffer-source",
				      "buffer", buffer, nullptr);

  
  GeglNode *save = gegl_node_new_child(graph,
				       "operation", "gegl:save",
				       "path", path.c_str(),
				       nullptr);  
  gegl_node_link_many(src, save, nullptr);
  gegl_node_process(save);
  g_object_unref(graph);
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  gegl_init(&argc, &argv);

  /* --------------------
     Use boot::program_options to do ARGV parsing
     -------------------- */
  
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "describe arguments")
    ("src", po::value<std::string>(), "src")
    ;
  po::variables_map options;
  po::store(po::parse_command_line(argc, argv, desc), options);
  po::notify(options);

  /* --------------------
     Hardcode values for the moment
     -------------------- */
  
  double x = 10;
  double y = 10;
  double w = 600;
  double h = 450;
  double l = 2.0;
  GeglColor *c = gegl_color_new("#FF0000");
  GeglNode *graph, *src;

  /* --------------------
     load image into buffer
     -------------------- */

  std::string src_path = options.count("src") ? options["src"].as<std::string>() : "test/286.jpg";
  GeglBuffer *buffer = pe_load(src_path);

  //  const GeglRectangle *rect = gegl_buffer_get_extent(buffer);
  //  printf("%d %d %d %d\n", rect->x, rect->y, rect->width, rect->height);

  /* --------------------
     Use buffer as source
     -------------------- */
  
  graph = gegl_node_new();
  src = gegl_node_new_child(graph, "operation", "gegl:buffer-source",
				      "buffer", buffer, nullptr);

  /* --------------------
     Create new path
     -------------------- */
  
  GeglPath *path = gegl_path_new();
  gegl_path_append(path, 'M', x, y);
  gegl_path_append(path, 'L', x + w, y);
  gegl_path_append(path, 'L', x + w, y + h);
  gegl_path_append(path, 'L', x, y + h);
  gegl_path_append(path, 'L', x, y);
  gegl_path_append(path, 'z');

  /* --------------------
     Stroke (and fill) path
     -------------------- */
  
  GeglNode *stroke = gegl_node_new_child(graph, "operation", "gegl:path",
					 "d", path,
					 "fill", c,
					 "fill-opacity", 0.5,
					 "stroke", c,
					 "stroke-width", l,
					 "stroke-hardness", 1.0,
					 nullptr);

  /* --------------------
     Use "over" composition to merge buffers
     -------------------- */
  
  GeglNode *over = gegl_node_new_child(graph, "operation", "gegl:over", nullptr);
  gegl_node_connect_to(stroke, "output", over, "aux");

  /* --------------------
     Write back to existing buffer
     -------------------- */
  
  GeglNode *write = gegl_node_new_child(graph, "operation", "gegl:write-buffer", "buffer", buffer, nullptr);
  
  gegl_node_link_many(src, over, write, nullptr);
  gegl_node_process(write);
  g_object_unref(graph);

  /* --------------------
     Save resulting image
     -------------------- */
  
  pe_save("foo.jpg", buffer);
  
  return 0;
}
