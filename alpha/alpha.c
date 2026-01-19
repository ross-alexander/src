#include <stdio.h>
#include <gegl.h>

/* ----------------------------------------------------------------------
   --
   -- image_mask
   --
   ---------------------------------------------------------------------- */

void image_mask(const char *path_image, const char *path_mask, const char *path_output)
{
  GeglBuffer *buffer_f, *buffer_b;
  GeglNode *graph, *load, *convert, *alpha, *sink, *over;

  /* Do color & format ahead of time */
  
  GeglColor *black = gegl_color_new ("rgb(0.0,0.0,0.0)");
  const Babl*cairo = babl_format("cairo-ARGB32");
  
  graph = gegl_node_new();

  /* Load mask and convert to alpha before storing in buffer */
  
  load = gegl_node_new_child(graph,
			     "operation", "gegl:load",
			     "path", path_mask,
			     0);
  convert = gegl_node_new_child(graph,
				"operation", "gegl:convert-format",
				"format", cairo,
				0);
  alpha = gegl_node_new_child(graph,
			      "operation", "gegl:color-to-alpha",
			      "color", black,
			      "transparency-threshold", 0.0,
			      "opacity-threshold", 1.0,
			      0);
  sink = gegl_node_new_child(graph,
			     "operation", "gegl:buffer-sink",
			     "buffer", &buffer_b,
			     0);

  gegl_node_link_many(load, convert, alpha, sink, 0);
  gegl_node_process(sink);
  g_object_unref(graph);

  /* load image into buffer */
  
  graph = gegl_node_new(); 
  load = gegl_node_new_child(graph, "operation", "gegl:load", "path", path_image, 0);
  convert = gegl_node_new_child(graph, "operation", "gegl:convert-format", "format", cairo, 0);
  sink = gegl_node_new_child(graph, "operation", "gegl:buffer-sink", "buffer", &buffer_f, 0);
  gegl_node_link_many(load, convert, sink, 0);
  gegl_node_process(sink);
  g_object_unref(graph);

  /* Merge buffers using multiply operation */
  
  graph = gegl_node_new();
  GeglNode *source_f = gegl_node_new_child(graph, "operation", "gegl:buffer-source", "buffer", buffer_f, 0);
  GeglNode *source_b = gegl_node_new_child(graph, "operation", "gegl:buffer-source", "buffer", buffer_b, 0);

  // Format check, not needed
  //  const Babl *f = gegl_buffer_get_format(buffer_b);
  //  printf("Format %s\n", babl_format_get_encoding(f));

  /* The correct operation is multiply */
  
  over = gegl_node_new_child(graph,
			     "operation", "gegl:multiply",
			     0);
  sink = gegl_node_new_child(graph,
			     "operation", "gegl:save",
			     "path", path_output,
			     0);
  
  gegl_node_connect(source_b, "output", over, "input");
  gegl_node_connect(source_f, "output", over, "aux");
  gegl_node_connect(over, "output", sink, "input");
  gegl_node_process(sink);
  g_object_unref(graph);
}  

/* ----------------------------------------------------------------------
   --
   -- main
   --
   ---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  /* color-to-alpha is GPL3 licensed so need to set application-licence */
  g_object_set(gegl_config(),
	       "application-license", "GPL3",
	       0);

  gegl_init(&argc, &argv);

  image_mask("/locker/gaming/cairn/tokens/rations-f.png",
	     "/locker/gaming/cairn/tokens/rations-b.png",
	     "/locker/gaming/cairn/tokens/rations.png");
  return 0;
}
