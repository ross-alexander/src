/* ----------------------------------------------------------------------

decode luajit bytecode (http://wiki.luajit.org/Bytecode-2.0)

Most of the defines have been lifted from lj_bc.h

luajit -bg t01.lua t01.raw

2021.1.4:
  - Table decode not finished

---------------------------------------------------------------------- */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Bytecode instruction format, 32 bit wide, fields of 8 or 16 bit:
**
** +----+----+----+----+
** | B  | C  | A  | OP | Format ABC
** +----+----+----+----+
** |    D    | A  | OP | Format AD
** +--------------------
** MSB               LSB
**
** In-memory instructions are always stored in host byte order.
*/

#define BCDUMP_VERSION   2

#define BCDUMP_F_BE		0x01
#define BCDUMP_F_STRIP		0x02
#define BCDUMP_F_FFI		0x04
#define BCDUMP_F_FR2		0x08

typedef uint32_t BCReg;  /* Bytecode register. */

#include "bc_ops.h"


#define MMDEF_FFI(_)
#define MMDEF_PAIRS(_)
#define MM_pairs        255
#define MM_ipairs       255

#define MMDEF(_) \
  _(index) _(newindex) _(gc) _(mode) _(eq) _(len) \
  /* Only the above (fast) metamethods are negative cached (max. 8). */ \
  _(lt) _(le) _(concat) _(call) \
  /* The following must be in ORDER ARITH. */ \
  _(add) _(sub) _(mul) _(div) _(mod) _(pow) _(unm) \
  /* The following are used in the standard libraries. */ \
  _(metatable) _(tostring) MMDEF_FFI(_) MMDEF_PAIRS(_)

typedef enum {
#define MMENUM(name)    MM_##name,
MMDEF(MMENUM)
#undef MMENUM
  MM__MAX,
  MM____ = MM__MAX,
  MM_FAST = MM_len
} MMS;

#define LJ_DATADEF

typedef enum {
  BCMnone, BCMdst, BCMbase, BCMvar, BCMrbase, BCMuv,  /* Mode A must be <= 7 */
  BCMlit, BCMlits, BCMpri, BCMnum, BCMstr, BCMtab, BCMfunc, BCMjump, BCMcdata,
  BCM_max
} BCMode;

#define BCM___          BCMnone

/* There are 14 modes, not including none and max, allow it to be
  packed into 4 bits.  Operand A can only be one of the first 7,
  allowing it to be packed into the first 3 bits.  The last 5 bits are
  the meta method, which there are 20, requiring a minimum of 5 bits.

  +-----+----+----+---+
  | MM  | C  | B  | A |
  +-----+----+----+---+
*/


#define BCMODE(name, ma, mb, mc, mm) (BCM##ma|(BCM##mb<<3)|(BCM##mc<<7)|(MM_##mm<<11)),
#define BCSTRUCT(mname, ma, mb, mc, mt)  {.name = #mname, .a = #ma, .b = #mb, .c = #mc, .t = #mt, .mode = (BCM##ma|(BCM##mb<<3)|(BCM##mc<<7)|(MM_##mt<<11))},

#define bcmode_a(op)    ((BCMode)(lj_bc_mode[op] & 7))
#define bcmode_b(op)    ((BCMode)((lj_bc_mode[op]>>3) & 15))
#define bcmode_c(op)    ((BCMode)((lj_bc_mode[op]>>7) & 15))
#define bcmode_d(op)    bcmode_c(op)
#define bcmode_hasd(op) ((lj_bc_mode[op] & (15<<3)) == (BCMnone<<3))
#define bcmode_mm(op)   ((MMS)(lj_bc_mode[op]>>11))

/* Macros to get instruction fields. */
#define bc_op(i)        ((BCOp)((i)&0xff))
#define bc_a(i)         ((BCReg)(((i)>>8)&0xff))
#define bc_b(i)         ((BCReg)((i)>>24))
#define bc_c(i)         ((BCReg)(((i)>>16)&0xff))
#define bc_d(i)         ((BCReg)((i)>>16))
#define bc_j(i)         ((ptrdiff_t)bc_d(i)-BCBIAS_J)

/* Bytecode opcode numbers. */
typedef enum {
#define BCENUM(name, ma, mb, mc, mt)    BC_##name,
BCDEF(BCENUM)
#undef BCENUM
  BC__MAX
} BCOp;


LJ_DATADEF const uint16_t lj_bc_mode[] = {
BCDEF(BCMODE)
};

struct op {
  char *name, *a, *b, *c, *t;
  uint16_t mode;
};

typedef union {
  double d;
  struct {
    uint32_t lo;
    uint32_t hi;
  } u;
} num;


struct op ops[] = {
BCDEF(BCSTRUCT)
};

/* Type codes for the GC constants of a prototype. Plus length for strings. */

enum {
  BCDUMP_KGC_CHILD, BCDUMP_KGC_TAB, BCDUMP_KGC_I64, BCDUMP_KGC_U64,
  BCDUMP_KGC_COMPLEX, BCDUMP_KGC_STR
};

/* Type codes for the keys/values of a constant table. */

enum {
  BCDUMP_KTAB_NIL, BCDUMP_KTAB_FALSE, BCDUMP_KTAB_TRUE,
  BCDUMP_KTAB_INT, BCDUMP_KTAB_NUM, BCDUMP_KTAB_STR
};


/* ----------------------------------------------------------------------
--
-- uleb128_decode
--
---------------------------------------------------------------------- */

uint32_t uleb128_decode(uint8_t **p)
{
  long result = 0;
  int shift = 0;
  while (1)
    {
      uint8_t byte = **p;
      (*p)++;
      result |= (byte & 0x7f) << shift;
      if ((byte & 0x80) == 0)
	break;
      shift += 7;
    }
  return result;
}

uint32_t uleb128_33_decode(uint8_t **p)
{
  uint32_t v = **p >> 1; (*p)++;
  if (v & 0x40)
    {
      int shift = 6;
      v &= 0x3f;
      while (1)
	{
	  uint8_t b = **p; (*p)++;
	  v |= (b & 0x7f) << shift;
	  if ((b & 0x80) == 0)
	    break;
	  shift += 7;
      }
    }
  return v;
}

/* ----------------------------------------------------------------------
--
-- decode_ktabk
--
---------------------------------------------------------------------- */
void decode_ktabk(uint8_t **pptr)
{
  int ktype = uleb128_decode(pptr);
  if (ktype == BCDUMP_KTAB_NIL)
    printf("nil\n");
  else if (ktype == BCDUMP_KTAB_INT)
    {
      int ktabv = uleb128_decode(pptr);
      printf("int(%d)\n", ktabv);
    }
  else if (ktype == BCDUMP_KTAB_NUM)
    {
      num n;
      n.u.lo = uleb128_decode(pptr);
      n.u.hi = uleb128_decode(pptr);
      printf("num(%f)\n", n.d);
    }
  else
    {
      exit(0);
    }
}


/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char* argv[])
{
  if (argc < 2)
    {
      fprintf(stderr, "%s: <bytecode file>\n", argv[0]);
      exit(1);
    }

  char *path = argv[1];
  
  FILE *stream;
  if ((stream = fopen(path, "r")) == 0)
    {
      fprintf(stderr, "%s: failed to open %s (%s)\n", argv[0], path, strerror(errno));
      exit(1);
    }

  // Determine size of file

  fseek(stream, 0L, SEEK_END);
  long code_size = ftell(stream);
  rewind(stream);

  uint8_t *code = calloc(sizeof(uint8_t), code_size);

  if (code_size != fread(code, sizeof(uint8_t), code_size, stream))
    {
      fprintf(stderr, "%s: tell/read size mismatch", argv[0]);
      exit(1);
    }

  /* Check header magic ESC L J 0x02 */
  
  uint8_t header[4] = {'\x1b', 'L', 'J', BCDUMP_VERSION};

  if (memcmp(code, header, sizeof(header) != 0))
    {
      fprintf(stderr, "%s: %s does not match header\n", argv[0], path);
      exit(1);
    }

  /*
dump   = header proto+ 0U
header = ESC 'L' 'J' versionB flagsU [namelenU nameB*]
proto  = lengthU pdata
pdata  = phead bcinsW* uvdataH* kgc* knum* [debugB*]
phead  = flagsB numparamsB framesizeB numuvB numkgcU numknU numbcU
         [debuglenU [firstlineU numlineU]]
kgc    = kgctypeU { ktab | (loU hiU) | (rloU rhiU iloU ihiU) | strB* }
knum   = intU0 | (loU1 hiU)
ktab   = narrayU nhashU karray* khash*
karray = ktabk
khash  = ktabk ktabk
ktabk  = ktabtypeU { intU | (loU hiU) | strB* }

B = 8 bit, H = 16 bit, W = 32 bit, U = ULEB128 of W, U0/U1 = ULEB128 of W+1
  */
  
  uint8_t *ptr = code + 4;
  long flags = uleb128_decode(&ptr);
  if (!(flags & BCDUMP_F_STRIP))
    {
      long name_len = uleb128_decode(&ptr);
      printf("%.*s\n", name_len, ptr);
      ptr += name_len;
    }

  /* Loop over proto structures */
  
  while (*ptr && ((ptrdiff_t)(ptr - code) < code_size))
    {
      long len = uleb128_decode(&ptr);

      /* Save ptr for the moment to add len to for next proto */

      uint8_t *ptr_save = ptr;
      printf("proto start = %d, len = %d\n", (ptr - code), len);

      int pflags = ptr[0];
      int numparams = ptr[1];
      int framesize = ptr[2];
      int numuv = ptr[3];
      ptr += 4;
      int numkgc = uleb128_decode(&ptr);
      int numkn = uleb128_decode(&ptr);
      int numbc = uleb128_decode(&ptr);

      printf("flags:%d params:%d framesize:%d uv:%d kgc:%d kn:%d bc:%d\n", pflags, numparams, framesize, numuv, numkgc, numkn, numbc);

      if (!(flags & BCDUMP_F_STRIP))
	{
	  int debug_len = uleb128_decode(&ptr);
	  int debug_line_first = uleb128_decode(&ptr);
	  int debug_line_num = uleb128_decode(&ptr);
	}

      /* Decode instructions */
      
      uint32_t *ins_ptr = (uint32_t*)ptr;
      ptr += numbc * sizeof(uint32_t);

      /* Up-values */
      for (int i = 0; i < numuv; i++)
	{
	  int uvdata = (ptr[1] << 8) | ptr[0]; ptr += 2;
	  printf("uvdata = %d\n", uvdata);
	}

      printf("kgc @ %d\n", (ptr - code));
      
      /* kgc */

      /* count strings to create array */
      
      int str_num = 0;
      char **strings = calloc(sizeof(char*), numkgc);
      
      for (int i = 0; i < numkgc; i++)
	{
	  int kgc_type = uleb128_decode(&ptr);
	  if (kgc_type > BCDUMP_KGC_STR)
	    {
	      int str_len = kgc_type - BCDUMP_KGC_STR;
	      strings[str_num] = strndup(ptr, str_len);
	      printf("kgc %d STR %s\n", i, strings[str_num]);
	      str_num++;
	       ptr += str_len;
	    }
	  else if (kgc_type == BCDUMP_KGC_TAB)
	    {
	      int narray = uleb128_decode(&ptr);
	      int nhash = uleb128_decode(&ptr);
	      printf("kgc %d TABLE %d %d\n", i, narray, nhash);

	      for (int i = 0; i < narray; i++)
		decode_ktabk(&ptr);
	      for (int i = 0; i < nhash; i++)
		{
		  decode_ktabk(&ptr);
		  decode_ktabk(&ptr);
		}
	    }
	  else
	    {
	      printf("kgc %d -- %d\n", i, kgc_type);
	    }
	}
      
      /* kn */

      printf("num constants @ %d: %d\n", (ptr - code), numkn);
      
      for (int i = 0; i < numkn; i++)
	{
	  num v;
	  int isnum = (ptr[0] & 1);
	  uint32_t lo = uleb128_33_decode(&ptr);
	  if (isnum)
	    {
	      v.u.hi = uleb128_decode(&ptr);
	      v.u.lo = lo;
	    }
	  else
	    {
	      v.d = lo;
	    }
	  printf("num %d: %f\n", v.d);
	}    
      ptr = ptr_save + len;

      printf("\nDecoded instructions\n\n");
      
      for (int i = 0; i < numbc; i++)
	{
	  uint32_t ins = ins_ptr[i];
	  uint32_t op = bc_op(ins);
	  printf("%03d: %08x %d %-8s", i, ins, op, ops[op].name);
	  printf("%-5s %-5s %-5s ", ops[op].a, ops[op].b, ops[op].c);
	  if (bcmode_hasd(op))
	    printf("%4d %4d     ", bc_a(ins), bc_d(ins));
	  else
	    printf("%4d %4d %4d", bc_a(ins), bc_b(ins), bc_c(ins));

	  if (bcmode_d(op) == BCMstr)
	    printf(" ; %s", strings[numkgc - 1 - (bcmode_hasd(op) ? bc_d(ins) : bc_c(ins))]);
	  
	  printf("\n");
	}
    }
  exit(0);
}
