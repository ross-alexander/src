#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>

// ret aka jalr x0, x1, 0
// = I-type 000000000000 00001 000 00000 1100111
// = 0x0 0x0 0x80 0x67

// Instruction is 0x00008067 but as system is little-endian in reverse order.

/* Because all instructions are 32 bits in length (excluding
   compressed instructions) there is no instruction to load a full 32
   bit immediate into a register.  To do this load upper immediate
   loads 20 bits into bits 31-12 of the register (regardless of XLEN)
   and then sign extends it if XLEN==64.

   The lower 12 bits can then be added, but because these are sign
   extended, if bit 11 is 1 then the number is treated as a negative
   number and sign extended before being added.  To fix this 1 needs
   to be added to upper 20 bits.
*/

//   lui a0, 0               = 0x00000537
//   addi a0, zero, 0        = 0x00001365
//   ret                     = 0x00008067
  
unsigned char code[] = {0x37, 0x05, 0x00, 0x00, 0x13, 0x65, 0x00, 0x00, 0x67, 0x80, 0x00, 0x00};

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
      fprintf(stderr, "Usage: jit1 <integer>\n");
      return 1;
    }

  int num = atoi(argv[1]);

  unsigned int *code32 = (unsigned int*)code;
    
    /* Set top 20 bits in U-type instruction */

  code32[0] |= (num | 0xfffff000);

  // Set top 12 bits in I-type instruction to lower 12 bits of num */

  printf("0x%08x 0x%08x\n", num, code32[1]);
  
  code32[1] |= (num << 20);

  printf("0x%08x\n", code32[1]);

  size_t size = sizeof(code);

  // RISCV doesn't like PROT_WRITE|PROT_EXEC without PROT_READ
  
  void *mem = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
  if (mem == (void*)-1)
    {
      printf("Error: %s\n", strerror(errno));
      exit(1);
    }
  
  memcpy(mem, code, sizeof(code));
  
  int (*func)() = mem;
  
  return func();
}
