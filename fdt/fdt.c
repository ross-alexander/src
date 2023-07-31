/* ----------------------------------------------------------------------
--
-- parse fdt Device Tree Binary (DTB)
--
---------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <libfdt_env.h>
#include <fdt.h>

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
      fprintf(stderr, "%s: <dtb file>\n", argv[0]);
      exit(1);
    }

  /* Open file using stdio */
  
  FILE *stream;
  if ((stream = fopen(argv[1], "rb")) == NULL)
    {
      fprintf(stderr, "%s: cannot open %s [%s]\n", argv[0], argv[1], strerror(errno));
      exit(1);
    }

  /* Read header */
  
  struct fdt_header* header = calloc(1, sizeof(struct fdt_header));
  fread(header, sizeof(struct fdt_header), 1, stream);

  /* Check MAGIC number */
  
  if (fdt32_to_cpu(header->magic) != FDT_MAGIC)
    {
      fprintf(stderr, "%s: %s failed to match magic [%04x]\n", argv[0], argv[1], FDT_MAGIC);
      exit(1);
    }

  fprintf(stdout, "version: %d\n", fdt32_to_cpu(header->version));
  fprintf(stdout, "last_comp_version: %d\n", fdt32_to_cpu(header->last_comp_version));

  size_t totalsize = fdt32_to_cpu(header->totalsize);

  /* Realloc and read in body */
  
  header = realloc(header, totalsize);
  fread(header + 1, totalsize - sizeof(struct fdt_header), 1, stream);

  printf("struct offset %d -- %02x\n", fdt32_to_cpu(header->off_dt_struct), fdt32_to_cpu(header->off_dt_struct));
  printf("string offset %d -- %02x\n", fdt32_to_cpu(header->off_dt_strings), fdt32_to_cpu(header->off_dt_strings));
  
  uint8_t *strings = (uint8_t*)((uint8_t*)header + fdt32_to_cpu(header->off_dt_strings));
    
  fdt32_t *nodeptr = (fdt32_t*)((uint8_t*)header + fdt32_to_cpu(header->off_dt_struct));

  uint32_t index = 0;
  int fin = 0;
  int level = 0;
  while(fin != 1)
    {
      fdt32_t nodetype = fdt32_to_cpu(nodeptr[index++]);
      for (int j = 0; j < (level<<2); j++) putchar(' ');
      switch(nodetype)
	{
	case FDT_BEGIN_NODE:
	  {
	    uint8_t *name = (uint8_t*)(nodeptr + index);
	    size_t len = strlen(name);
	    printf("%s {\n", name);
	    index += ((len) >> 2) + 1;
	    level++;
	    break;
	  }
	case FDT_PROP:
	  {
	    fdt32_t len = fdt32_to_cpu(nodeptr[index + 0]);
	    fdt32_t off = fdt32_to_cpu(nodeptr[index + 1]);
	    printf("%s [%d]\n", strings + off, len);
	    index += ((len+3) >> 2) + 2;
	    break;
	  }
	case FDT_END_NODE:
	  {
	    printf("}\n");
	    level--;
	    break;
	  }
	case FDT_END:
	  {
	    fin = 1;
	    break;
	  }
	default:
	  break;
	}
    }
}
