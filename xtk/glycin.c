#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <cairo.h>
#include <glycin.h>
#include <babl/babl.h>

#include "common.h"

const char* id(void)
{
  return "glycin";
}

void init(gint *argc, char ***argv)
{
  printf("glycin-init\n");
  babl_init();
}

xtk_t* create(int w, int h, char *path)
{
  xtk_t *xtk = calloc(1, sizeof(xtk_t));
  xtk->width = w;
  xtk->height = h;

  GError *error;

  GFile *file = g_file_new_for_path(path);

  GlyLoader *loader = gly_loader_new(file);

  /* --------------------
     -- Load image and return 0 if fails
     -------------------- */
  
  GlyImage *image = gly_loader_load(loader, &error);
  if (image == nullptr)
    {
      fprintf(stderr, "Failed to load %s: %s\n", path, error->message);
      return 0;
    }
  
  printf("Image %s loaded\n", path);

  /* --------------------
     Get first and probably only frame, exit if fails
     -------------------- */
  
  GlyFrame *frame = gly_image_next_frame(image, &error);

  if (frame == nullptr)
    {
      fprintf(stderr, "Failed to load frame: %s\n", error->message);
      g_object_unref(image);
      g_object_unref(loader);
      return 0;
    }

  /* --------------------
     Get format and create Babl
     -------------------- */
  
  GlyMemoryFormat format = gly_frame_get_memory_format(frame);
  char *description = nullptr;
  const Babl *babl_format_src = nullptr;
  
  switch(format)
    {
    case GLY_MEMORY_B8G8R8A8_PREMULTIPLIED:
      printf("8-bit RGRA premultiplied.\n");
      break;
      
    case GLY_MEMORY_A8R8G8B8_PREMULTIPLIED:
      printf("8-bit ARGB premultiplied.\n");
      break;
      
    case GLY_MEMORY_R8G8B8A8_PREMULTIPLIED:
      printf("8-bit RGBA premultiplied.\n");
      break;
      
    case GLY_MEMORY_B8G8R8A8:
      printf("8-bit RGBA.\n");
      break;
    case GLY_MEMORY_A8R8G8B8:
      printf("8-bit AGBR.\n");
      break;
    case GLY_MEMORY_R8G8B8A8:
      printf("8-bit RGBA.\n");
      break;
    case GLY_MEMORY_A8B8G8R8:
      printf("8-bit ABGR.\n");
      break;
    case GLY_MEMORY_R8G8B8:
      babl_format_src = babl_format_with_space("R'G'B' u8", NULL);
      description = "8-bit RGB";
      break;
    case GLY_MEMORY_B8G8R8:
      printf("8-bit BGR.\n");
      break;
    case GLY_MEMORY_R16G16B16:
      printf("16-bit RGB.\n");
      break;
    case GLY_MEMORY_R16G16B16A16_PREMULTIPLIED:
      printf("16-bit RGBA premultiplied.\n");
      break;
    case GLY_MEMORY_R16G16B16A16:
      printf("16-bit RGBA.\n");
      break;
    case GLY_MEMORY_R16G16B16_FLOAT:
      printf("16-bit float RGB.\n");
      break;
    case GLY_MEMORY_R16G16B16A16_FLOAT:
      printf("16-bit float RGBA.\n");
      break;
    case GLY_MEMORY_R32G32B32_FLOAT:
      printf("32-bit float RGB.\n");
      break;
    case GLY_MEMORY_R32G32B32A32_FLOAT_PREMULTIPLIED:
      printf("32-bit float RGBA premultiplied.\n");
      break;
    case GLY_MEMORY_R32G32B32A32_FLOAT:
      printf("16-bit float RGBA.\n");
      break;
    case GLY_MEMORY_G8A8_PREMULTIPLIED:
      printf("8-bit gray with alpha premultiplied.\n");
      break;
    case GLY_MEMORY_G8A8:
      printf("8-bit gray with alpha.\n");
      break;
    case GLY_MEMORY_G8:
      printf("8-bit gray.\n");
      break;
    case GLY_MEMORY_G16A16_PREMULTIPLIED:
      printf("16-bit gray with alpha premultiplied.\n");
      break;
    case GLY_MEMORY_G16A16:
      printf("16-bit gray with alpha.\n");
      break;
    case GLY_MEMORY_G16:
      printf("16-bit gray.\n");
      break;
    default:
      printf("Unknown format is %d\n", format);
      break;
    }

  uint32_t height = gly_frame_get_height(frame);
  uint32_t width = gly_frame_get_width(frame);
  uint32_t stride_src = gly_frame_get_stride(frame);
  gboolean alpha = gly_memory_format_has_alpha(format);

  if (babl_format_src)
    {
      GBytes *src = gly_frame_get_buf_bytes(frame);
      gsize buffer_src_size;
      gconstpointer buffer_src = g_bytes_get_data(src, &buffer_src_size);
      printf("Size should be %d, has %ld\n", height * stride_src, buffer_src_size);
      
      cairo_format_t cairo_format = alpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24;

      uint32_t stride_dst = cairo_format_stride_for_width(cairo_format, width);

      void *buffer_dst = malloc(stride_dst * height);
      
      cairo_surface_t* image = cairo_image_surface_create_for_data(buffer_dst, cairo_format, width, height, stride_dst);
      const Babl *babl_format_dst = alpha ? babl_format("cairo-ARGB32") : babl_format("cairo-RGB24");

      printf("Converting from %s [%s] format %s [%d] to %s [%d]\n", description, alpha ? "alpha" : "no alpha",
	     babl_get_name(babl_format_src),
	     babl_format_get_bytes_per_pixel(babl_format_src),
	     babl_get_name(babl_format_dst),
	     babl_format_get_bytes_per_pixel(babl_format_dst)
	     );

      printf("Image width [%d] height [%d]\n", width, height);
      
      printf("Source stride [%d] Destination stride [%d]\n", stride_src, stride_dst);
      
      babl_process_rows(babl_fish(babl_format_src, babl_format_dst),
			buffer_src,
			stride_src,
			buffer_dst,
			stride_dst,
			width,
			height);

      cairo_surface_mark_dirty(image);
      xtk->width = width;
      xtk->height = height;
      xtk->source = image;
    }
  g_object_unref(frame);
  g_object_unref(image);
  g_object_unref(loader);
  return xtk;
}
