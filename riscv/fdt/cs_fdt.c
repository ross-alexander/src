/* ----------------------------------------------------------------------
   --
   -- fdt
   --
   -- 2022-11-03: Clean sheet FDT parser
   --
   ---------------------------------------------------------------------- */

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <endian.h>
#include <assert.h>

/* https://github.com/devicetree-org/devicetree-specification/releases/tag/v0.4-rc1 */

struct cs_fdt_header {
  uint32_t magic;
  uint32_t totalsize;
  uint32_t off_dt_struct;
  uint32_t off_dt_strings;
  uint32_t off_mem_rsvmap;
  uint32_t version;
  uint32_t last_comp_version;
  uint32_t boot_cpuid_phys;
  uint32_t size_dt_strings;
  uint32_t size_dt_struct;
};

enum {
  FDT_BEGIN_NODE = 1,
  FDT_END_NODE,
  FDT_PROP,
  FDT_NOP,
  FDT_END
};

typedef uint16_t cs_fdt16_t;
typedef uint32_t cs_fdt32_t;
typedef uint64_t cs_fdt64_t;

#define FDT_MAGIC 0xd00dfeed

static inline uint16_t fdt16_to_cpu(cs_fdt16_t x)
{
  return be16toh(x);
}
static inline cs_fdt16_t cpu_to_fdt16(uint16_t x)
{
  return htobe16(x);
}

static inline uint32_t fdt32_to_cpu(cs_fdt32_t x)
{
  return be32toh(x);
  }
static inline cs_fdt32_t cpu_to_fdt32(uint32_t x)
{
  return htobe32(x);
}

static inline uint64_t fdt64_to_cpu(cs_fdt64_t x)
{
  return be64toh(x);
}
static inline cs_fdt64_t cpu_to_fdt64(uint64_t x)
{
  return htobe64(x);
}

struct dt_bits {
  unsigned int address_cells;
  unsigned int size_cells;
};

/* ----------------------------------------------------------------------
   --
   -- cs_dt_find
   --
   ---------------------------------------------------------------------- */

int cs_dt_find(cs_fdt32_t *ptr, cs_fdt32_t size, const char *strings, const char *name)
{
  cs_fdt32_t index = 0;
  const char *nodename = 0;
  for ( ; index<<2 < size; index++)
    {
      cs_fdt32_t token = fdt32_to_cpu(ptr[index]);
      //      printf("index %d size %d token %d\n", index, size, token);
  
      switch(token)
	{
	case FDT_BEGIN_NODE:
	  {
	    nodename = (char*)&ptr[index+1];
	    printf("*** %s\n", nodename);
	    cs_fdt32_t len = strlen(nodename);
	    cs_fdt32_t block_len = 1 + ((len + 1 + 3) >> 2);
	    cs_fdt32_t res = cs_dt_find(ptr + index + block_len, size - (block_len<<2), strings, name) + block_len;;
	    index += res;
	    break;
	  }
	case FDT_PROP:
	  {
	    cs_fdt32_t len = fdt32_to_cpu(ptr[index + 1]);
	    cs_fdt32_t off = fdt32_to_cpu(ptr[index + 2]);
	    
	    const unsigned char *prop_key = strings + off;
	    const unsigned char *prop_value = (const unsigned char*) &ptr[index + 3];
	    cs_fdt32_t block_len = 2 + ((len + 3) >> 2);

	    //	    printf("-- %d %d %s\n", index, block_len, prop_key);

	    if (strcmp("#address-cells", prop_key) == 0)
	      {
		//		local_bits.address_cells = fdt32_to_cpu(ptr[index + 3]);
		//		printf(": %d", local_bits.address_cells);
	      }
	    if (strcmp("#size-cells", prop_key) == 0)
	      {
		//		local_bits.size_cells = fdt32_to_cpu(ptr[index + 3]);
		//		printf(": %d", local_bits.size_cells);
	      }
	    if (strcmp("compatible", prop_key) == 0)
	      {
		const unsigned char *s = prop_value;
		unsigned int len_acc = 0;
		printf(":");
		while (len_acc < len)
		  {
		    if (strcmp(name, s) == 0)
		      printf(" + %s", s);
		    else
		      printf(" - %s", s);
		    
		    len_acc += strlen(s) + 1;
		    s += strlen(s)+1;
		  }
		printf("\n");
	      }
	    index += block_len;
	    break;
	  }
	case FDT_NOP:
	  {
	    break;
	  }
	case FDT_END_NODE:
	  {
	    return index;
	    break;
	  }
	case FDT_END:
	  {
	    return index;
	    break;
	  }
	}
    }
  return 0;
}


/* ----------------------------------------------------------------------
   --
   -- cs_dt_parse
   --
   ---------------------------------------------------------------------- */

int cs_dt_parse(cs_fdt32_t *ptr, cs_fdt32_t size, const char *strings, int level, const char *path, struct dt_bits *bits)
{
  cs_fdt32_t index = 0;
  struct dt_bits local_bits;

  memcpy(&local_bits, bits, sizeof(struct dt_bits));
  
  for ( ; index<<2 < size; index++)
    {
      cs_fdt32_t token = fdt32_to_cpu(ptr[index]);
      //      printf("index %d size %d token %d\n", index, size, token);
  
      switch(token)
	{
	case FDT_BEGIN_NODE:
	  {
	    const char *name = (char*)&ptr[index+1];
	    cs_fdt32_t len = strlen(name);
	    
	    for (int i = 0; i < level; i++)
	      printf("  ");

	    const char *path_sep = "/";
	    
	    char *extended_path = 0;

	    // The path is / for root, /node for level 1 and /node1/node2 for subsequent levels

	    // The root name is '', so will have length 0
	    if(strlen(name) == 0)
	      {
		if (path != 0)
		  {
		    assert("Error: name == '' and path != null");
		  }
		extended_path = (char*)path_sep;
	      }
	    else
	      {
		unsigned int plen = strlen(path) + strlen(name) + 2;
		extended_path = alloca(plen);
		strcpy(extended_path, path);
		if (strcmp(path, path_sep) == 0)
		  {
		    strcpy(extended_path + strlen(path), name);
		  }
		else
		  {
		    strcpy(extended_path + strlen(path), path_sep);
		    strcpy(extended_path + strlen(path) + 1, name);
		  }
	      }
	    printf("%s [%s]\n", extended_path, name);

	    cs_fdt32_t block_len = 1 + ((len + 1 + 3) >> 2);
	    //	    printf("blocklen %d\n", block_len);
	    cs_fdt32_t res = cs_dt_parse(ptr + index + block_len, size - (block_len<<2), strings, level+1, extended_path, &local_bits) + block_len;
	    index += res;
	    break;
	  }
	case FDT_PROP:
	  {
	    cs_fdt32_t len = fdt32_to_cpu(ptr[index + 1]);
	    cs_fdt32_t off = fdt32_to_cpu(ptr[index + 2]);
	    
	    for (int i = 0; i < level; i++)
	      printf("  ");

	    const unsigned char *prop_key = strings + off;
	    const unsigned char *prop_value = (const unsigned char*) &ptr[index + 3];
	    cs_fdt32_t block_len = 2 + ((len + 3) >> 2);
	    printf("%s", prop_key);

	    if (strcmp("#address-cells", prop_key) == 0)
	      {
		local_bits.address_cells = fdt32_to_cpu(ptr[index + 3]);
		printf(": %d", local_bits.address_cells);
	      }
	    if (strcmp("#size-cells", prop_key) == 0)
	      {
		local_bits.size_cells = fdt32_to_cpu(ptr[index + 3]);
		printf(": %d", local_bits.size_cells);
	      }
	    if (strcmp("reg", prop_key) == 0)
	      {
		int cell_count = len / sizeof(cs_fdt32_t);
		printf(":");
		for (int cell = 0; cell < cell_count; cell++)
		  printf(" 0x%08x", fdt32_to_cpu(ptr[index + 3 + cell]));
	      }
	    if (strcmp("compatible", prop_key) == 0)
	      {
		const unsigned char *s = prop_value;
		unsigned int len_acc = 0;
		printf(":");
		while (len_acc < len)
		  {
		    printf(" %s", s);
		    len_acc += strlen(s) + 1;
		    s += strlen(s)+1;
		  }
	      }
	    if (strcmp("device_type", prop_key) == 0)
	      printf(": %s", prop_value);
	    
	    printf("\n");
	    //	    printf("blocklen %d\n", block_len);
	    index += block_len;
	    break;
	  }
	case FDT_NOP:
	  {
	    break;
	  }
	case FDT_END_NODE:
	  {
	    return index;
	    break;
	  }
	case FDT_END:
	  {
	    return index;
	    break;
	  }
	}
    }
  return 0;
}


/* ----------------------------------------------------------------------
   --
   -- main
   --
   ---------------------------------------------------------------------- */

int main(int argc, const char *argv[])
{
  if (argc < 2)
    {
      fprintf(stderr, "%s: <DTB file>\n", argv[0]);
      exit(0);
    }
  const char *path = argv[1];

  /* stat file to get size for buffer */
  
  struct stat statbuf;
  if (stat(path, &statbuf) != 0)
    {
      fprintf(stderr, "%s: failed to stat %s: %s\n", argv[0], path, strerror(errno));
      exit(1);
    }

  void *buffer = malloc(statbuf.st_size);

  /* Open file and read into buffer */
  
  FILE *stream;
  if ((stream = fopen(path, "r")) == 0)
    {
      fprintf(stderr, "%s: failed to open %s: %s\n", argv[0], path, strerror(errno));
      exit(1);
    }
  if (fread(buffer, statbuf.st_size, 1, stream) != 1)
    {
      fprintf(stderr, "%s: failed to read %s: %s\n", argv[0], path, strerror(errno));
      exit(1);
    }

  /* Check header and version */

  struct cs_fdt_header *header = (struct cs_fdt_header*)buffer;

    if (fdt32_to_cpu(header->magic) != FDT_MAGIC)
    {
      printf("Cannot find FDT match magic [%04x]\n", FDT_MAGIC);
      exit(1);
    }

    struct dt_bits bits = {.address_cells = 0, .size_cells = 0};
    
    /* cs_dt_parse(cs_fdt32_t *ptr, cs_fdt32_t size, const char *strings, int level, const char *path) */

    cs_dt_parse((cs_fdt32_t*)((uint8_t*)buffer + fdt32_to_cpu(header->off_dt_struct)), fdt32_to_cpu(header->size_dt_struct), (char*)buffer + fdt32_to_cpu(header->off_dt_strings), 0, 0, &bits);

    cs_dt_find((cs_fdt32_t*)((uint8_t*)buffer + fdt32_to_cpu(header->off_dt_struct)), fdt32_to_cpu(header->size_dt_struct), (char*)buffer + fdt32_to_cpu(header->off_dt_strings), "ns16550a");
}
