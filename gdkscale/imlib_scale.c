#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <regex.h>

#include <Imlib2.h>
#include <babl/babl.h>
#include <cairo/cairo.h>

#include "path_split.h"

/* ----------------------------------------------------------------------
   --
   -- imlib_cairo_babl_convert
   --
   ---------------------------------------------------------------------- */

cairo_surface_t *imlib_cairo_babl_convert(Imlib_Image img)
{
  // Set the current image context
  imlib_context_set_image(img);

  // Retrieve original dimensions
  int width = imlib_image_get_width();
  int height = imlib_image_get_height();

  
  /* --------------------
     Use babl to convert to cairo ARGB32
     -------------------- */
  
  const Babl *src_format, *dst_format;
  cairo_format_t cairo_format;

  /* RGB with Tone Reponse Curve (aka gamma correction) from source */
  
  src_format = babl_format_new(babl_model("R'G'B'A"), babl_type("u8"),
			       babl_component("B'"),
			       babl_component("G'"),
			       babl_component("R'"),
			       babl_component("A"),
			       nullptr);


  /* Default to having alpha */
  
  cairo_format = CAIRO_FORMAT_ARGB32;
  dst_format = babl_format("cairo-ARGB32");
      
  cairo_surface_t *surface = cairo_image_surface_create(cairo_format, width, height);
  
  babl_process_rows(babl_fish(src_format, dst_format),
		    imlib_image_get_data_for_reading_only(),
		    width * 4,
		    cairo_image_surface_get_data(surface),
		    cairo_image_surface_get_stride(surface),
		    width,
		    height);

  return surface;
}

/* ----------------------------------------------------------------------
   --
   -- image_scale
   --
   ---------------------------------------------------------------------- */

int image_scale(int cairo, double scale, const char *format, const char *inpath, const char *outpath)
{
  struct path_ext_t* ext = path_split(inpath);

  if (!ext->file)
    {
      fprintf(stderr, "No filename in path\n");
      exit(1);
    }
  
  // 1. Load the original image
  Imlib_Image img = imlib_load_image(inpath);
  if (!img)
    {
      printf("Failed to load %s\n", inpath);
      return 1;
    }
  
  // Set the current image context
  imlib_context_set_image(img);

  // Retrieve original dimensions
  int width = imlib_image_get_width();
  int height = imlib_image_get_height();

  printf("Original size: %d × %d\n", width, height);


  if (cairo)
    {
      cairo_surface_t *surface = imlib_cairo_babl_convert(img);

      const char *save_ext = "-cairo.png";
      char *save_path = calloc(sizeof(char), strlen(ext->file) + strlen(save_ext) + 1);
      snprintf(save_path, strlen(ext->file) + strlen(save_ext) + 1, "%s%s", ext->file, save_ext);
      cairo_surface_write_to_png(surface, save_path);
      free(save_path);
      cairo_surface_destroy(surface);
    }

  /* --------------------
     Scale the image
     -------------------- */
  
  int new_w = (int)((double)width * scale);
  int new_h = (int)((double)height * scale);

  printf("Scaled size: %d × %d\n", new_w, new_h);
  
  Imlib_Image scaled_img = imlib_create_cropped_scaled_image(0, 0, width, height, new_w, new_h);

  if (scaled_img)
    {
      // Free the original and set context to the scaled image
      imlib_free_image();
      imlib_context_set_image(scaled_img);
    }
  
  // 3. Save the resulting image
  
  imlib_image_set_format(format);


  char *save_path;
  
  if (outpath)
    {
      save_path = strdup(outpath);
    }
  else
    {
      save_path = calloc(sizeof(char), strlen(ext->file) + strlen(format) + 2);
      snprintf(save_path, strlen(ext->file) + strlen(format) + 2, "%s.%s", ext->file, format);
    }
  
  imlib_image_attach_data_value("quality", NULL, 90, NULL);
  imlib_image_attach_data_value("compression", NULL, 8, NULL);
  
  imlib_save_image(save_path);
  free(save_path);
  
  // 4. Clean up memory

  imlib_free_image();
  printf("Image scaled and saved successfully.\n");
  return 1;
}  


/* ----------------------------------------------------------------------
   --
   -- main
   --
   ---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  babl_init();

  int ch;
  const char *format = "jpeg", *inpath = nullptr, *outpath = nullptr;
  double scale = 1.0;
  int cairo = 0;
  
  while ((ch = getopt(argc, argv, "cs:f:i:o:")) != EOF)
    switch(ch)
      {
      case 'f':
 	format = optarg;
	break;
      case 'c':
	cairo = 1;
	break;
      case 's':
	scale = strtod(optarg, nullptr);
	break;
      case 'i':
	inpath = optarg;
	break;
      case 'o':
	outpath = optarg;
	break;
      }

  if (((argc - optind) < 1) && !inpath)
    {
      printf("%s: <image file> | -i <image file>\n", argv[0]);
      return 0;
    }

  if (!inpath)
    inpath = argv[optind];

  image_scale(cairo, scale, format, inpath, outpath);
  return 0;
}
