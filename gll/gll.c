/* ----------------------------------------------------------------------

   2025-11-12: Glycin loader test C program

   ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#include <glycin.h>

void gll_load_path(const char *path)
{
  GFile *file = g_file_new_for_path (path);
  GlyLoader *loader = gly_loader_new (file);
  GlyImage *image = gly_loader_load (loader, NULL);
  if (image)
    {
      printf("Image %s loaded\n", path);
      GError *error;
      GlyFrame *frame = gly_image_next_frame(image, &error);
      GlyMemoryFormat format = gly_frame_get_memory_format(frame);

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
	  printf("8-bit RGB.\n");
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
	  printf("Format is %d\n", format);
	  break;
	}
    }
}

int main(int argc, char *argv[])
{
  char *path = 0;
  if (argc < 2)
    exit(0);
  for (unsigned int i = 1; i < argc; i++)
    gll_load_path(argv[i]);
  exit(0);
}
