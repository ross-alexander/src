/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */


/* ----------------------------------------------------------------------
--
-- Slightly home baked FDT code
--
---------------------------------------------------------------------- */

#include <stdint.h>
#include <sbi/sbi_ecall_interface.h>

#include "localsbi.h"

#ifdef __CHECKER__
#define FDT_FORCE __attribute__((force))
#define FDT_BITWISE __attribute__((bitwise))
#else
#define FDT_FORCE
#define FDT_BITWISE
#endif

typedef uint16_t FDT_BITWISE fdt16_t;
typedef uint32_t FDT_BITWISE fdt32_t;
typedef uint64_t FDT_BITWISE fdt64_t;

/* --------------------
-- FDT is big-endian but RISC-V is little endian by default
-------------------- */

#define EXTRACT_BYTE(x, n)	((unsigned long long)((uint8_t *)&x)[n])
#define CPU_TO_FDT16(x) ((EXTRACT_BYTE(x, 0) << 8) | EXTRACT_BYTE(x, 1))
#define CPU_TO_FDT32(x) ((EXTRACT_BYTE(x, 0) << 24) | (EXTRACT_BYTE(x, 1) << 16) | \
			 (EXTRACT_BYTE(x, 2) << 8) | EXTRACT_BYTE(x, 3))
#define CPU_TO_FDT64(x) ((EXTRACT_BYTE(x, 0) << 56) | (EXTRACT_BYTE(x, 1) << 48) | \
			 (EXTRACT_BYTE(x, 2) << 40) | (EXTRACT_BYTE(x, 3) << 32) | \
			 (EXTRACT_BYTE(x, 4) << 24) | (EXTRACT_BYTE(x, 5) << 16) | \
			 (EXTRACT_BYTE(x, 6) << 8) | EXTRACT_BYTE(x, 7))

static inline uint16_t fdt16_to_cpu(fdt16_t x)
{
	return (FDT_FORCE uint16_t)CPU_TO_FDT16(x);
}
static inline fdt16_t cpu_to_fdt16(uint16_t x)
{
	return (FDT_FORCE fdt16_t)CPU_TO_FDT16(x);
}

static inline uint32_t fdt32_to_cpu(fdt32_t x)
{
	return (FDT_FORCE uint32_t)CPU_TO_FDT32(x);
}
static inline fdt32_t cpu_to_fdt32(uint32_t x)
{
	return (FDT_FORCE fdt32_t)CPU_TO_FDT32(x);
}

static inline uint64_t fdt64_to_cpu(fdt64_t x)
{
	return (FDT_FORCE uint64_t)CPU_TO_FDT64(x);
}
static inline fdt64_t cpu_to_fdt64(uint64_t x)
{
	return (FDT_FORCE fdt64_t)CPU_TO_FDT64(x);
}
#undef CPU_TO_FDT64
#undef CPU_TO_FDT32
#undef CPU_TO_FDT16
#undef EXTRACT_BYTE

// #include <libfdt_env.h>

#include <fdt.h>

#include "printf/printf.h"

/* --------------------
-- Wait For Interrupt
-------------------- */

#define wfi()                                             \
	do {                                              \
		__asm__ __volatile__("wfi" ::: "memory"); \
	} while (0)


/* --------------------
Not required as passed to test_main as parameters
-------------------- */


// extern long _boot_a0;
// extern long _boot_a1;

/* ----------------------------------------------------------------------
--
-- string functions copied from openSBI to avoid C library
--
---------------------------------------------------------------------- */

int test_strcmp(const char *a, const char *b)
{
  /* search first diff or end of string */
  for (; *a == *b && *a != '\0'; a++, b++)
    ;
  return *a - *b;
}

int test_strncmp(const char *a, const char *b, size_t count)
{
  /* search first diff or end of string */
  for (; count > 0 && *a == *b && *a != '\0'; a++, b++, count--)
    ;
  
  return *a - *b;
}

size_t test_strlen(const char *str)
{
  unsigned long ret = 0;
  while (*str != '\0')
    {
      ret++;
      str++;
    }
  return ret;
}

/* ----------------------------------------------------------------------
--
-- fdt_dump
--
---------------------------------------------------------------------- */

void fdt_dump(struct fdt_header *fdt)
{
  printf("version: %d\n", fdt32_to_cpu(fdt->version));
  size_t totalsize = fdt32_to_cpu(fdt->totalsize);
  uint8_t *strings = (uint8_t*)((uint8_t*)fdt + fdt32_to_cpu(fdt->off_dt_strings));
  fdt32_t *nodeptr = (fdt32_t*)((uint8_t*)fdt + fdt32_to_cpu(fdt->off_dt_struct));

  uint32_t index = 0;
  int fin = 0;
  int level = 0;
  int indent = 4;

  while((!fin) && ((index * sizeof(fdt32_t)) < totalsize))
    {
      fdt32_t nodetype = fdt32_to_cpu(nodeptr[index++]);
      //      printf("[%d] %d\n", index, nodetype);
      for (int j = 0; j < (level * indent); j++) printf(" ");
      switch(nodetype)
	{
	case FDT_BEGIN_NODE:
	  {
	    char *name = (char*)&(nodeptr[index]);
	    size_t len = test_strlen(name);
	    printf("%s {\n", name);

/* len is actually len+1 because the string is nul terminated.
   Because nodetype is aligned len(+1)+3 is divided by 4 (>>2).
*/
	    index += ((len + 1 + 3) >> 2);
	    level++;
	    break;
	  }
	case FDT_PROP:
	  {
	    fdt32_t len = fdt32_to_cpu(nodeptr[index++]);
	    fdt32_t off = fdt32_to_cpu(nodeptr[index++]);
	    const char *prop = (const char*)strings + off;
	    printf("%s [%d]", prop, len);

	    /* --------------------
	       Nieve code for checking sring properties
	       -------------------- */

	    if (test_strcmp(prop, "compatible") == 0)
	      {
		char *value = (char*)&(nodeptr[index]);
		printf(" %s", value);
	      }
	    printf("\n");
	    /* len here is for binary data so we align it up to a 4 byte boundary */
	    index += ((len+3) >> 2);
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

/* ----------------------------------------------------------------------
--
-- test_main
--
---------------------------------------------------------------------- */

void test_main(unsigned long a0, unsigned long a1)
{

  /* --------------------
     Called by __start in test_main.S
     a0 is the hart lottery winner
     a1 is start of the FDT passed by SBI
     -------------------- */

  printf("\nTest payload running %s %s\n", __DATE__, __TIME__);

  printf("FDT@%08lx HART %08lx\n", a1, a0);
  	
  struct fdt_header* header = (struct fdt_header*)a1;
  
  if (fdt32_to_cpu(header->magic) == FDT_MAGIC)
    {
      printf("Found FDT match magic [%04x]\n", FDT_MAGIC);
      fdt_dump(header);
    }
  shutdown();
}
