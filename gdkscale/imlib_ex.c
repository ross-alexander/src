#include <Imlib2.h>
#include <stdio.h>
#include <babl/babl.h>
#include <cairo/cairo.h>

int main(int argc, char *argv[])
{
  babl_init();
  
  if (argc < 2)
    {
      printf("%s: <image file>\n", argv[0]);
      return 0;
    }

  const char *path = argv[1];

  // 1. Load the original image
  Imlib_Image img = imlib_load_image(path);
  if (!img)
    {
      printf("Failed to load %s\n", path);
      return 1;
    }
  
  // Set the current image context
  imlib_context_set_image(img);

  // Retrieve original dimensions
  int width = imlib_image_get_width();
  int height = imlib_image_get_height();
  printf("Original size: %dx%d\n", width, height);


  /* --------------------
     Use babl to convert to cairo ARGB32
     -------------------- */
  
  const Babl *src_format, *dst_format;
  cairo_format_t cairo_format;
  
  src_format = babl_format_new(babl_model("R'G'B'A"), babl_type("u8"),
			       babl_component("B'"),
			       babl_component("G'"),
			       babl_component("R'"),
			       babl_component("A"),
			       nullptr);

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

  /* Save to PNG */
  
  cairo_surface_write_to_png(surface, "babl.png");

  // 2. Scale the image (e.g., to 50% of its size)
  
  int new_w = width / 2;
  int new_h = height / 2;
  Imlib_Image scaled_img = imlib_create_cropped_scaled_image(0, 0, width, height, new_w, new_h);
  

  if (scaled_img)
    {
      // Free the original and set context to the scaled image
      imlib_free_image();
      imlib_context_set_image(scaled_img);
    }
  
  // 3. Save the resulting image
  
  imlib_image_set_format("png");
  imlib_save_image("output_scaled.png");
  
  // 4. Clean up memory

  imlib_free_image();
  printf("Image scaled and saved successfully.\n");
  
  return 0;
}
