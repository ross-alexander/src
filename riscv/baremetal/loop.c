#include "printf.h"

static const char *str = "looper\n";

typedef unsigned int uint32_t;
typedef unsigned int fdt32_t;
typedef unsigned char uint8_t;

extern long _boot_a0;
extern long _boot_a1;

static uint8_t* _uart_base;

void _putchar(char c)
{
  *(volatile char*)_uart_base = c;
}

static inline uint32_t fdt32_ld(const fdt32_t *p)
{
        const uint8_t *bp = (const uint8_t *)p;
        return ((uint32_t)bp[0] << 24)
                | ((uint32_t)bp[1] << 16)
                | ((uint32_t)bp[2] << 8)
                | bp[3];
}

static inline void fdt32_st(void *property, uint32_t value)
{
        uint8_t *bp = (uint8_t *)property;
        bp[0] = value >> 24;
        bp[1] = (value >> 16) & 0xff;
        bp[2] = (value >> 8) & 0xff;
        bp[3] = value & 0xff;
}

unsigned char v2a(unsigned char x)
{
  return (x < 10 ? '0'+x : 'a'-10+x);
}

void putlong(unsigned char *p, long n)
{
  int octets = sizeof(n);
  
  for (int i = 0; i < octets; i++)
    {
      int shift = (octets-(i+1)) << 3;
      unsigned long v = (n >> shift) & 0xff;
      //      putchar(v2a(i));
      //      putchar(' ');
      _putchar(v2a(v >> 4));
      _putchar(v2a(v & 0x0f));
      //      putchar('\n');
    }
  //  putchar('\n');
}

uint8_t* _puts(uint8_t *s)
{
  unsigned int i;
  for (i = 0; s[i]; i++)
    *_uart_base = s[i];
  return s + i;
}

/* ----------------------------------------------------------------------
   --
   -- _start
   --
   ---------------------------------------------------------------------- */

void _start(int hartid, void *ftd_base)
{
  /* The virt has NS16550 serial device */
  /* This should be picked up from the device tree but for now hardwire it */
  
  _uart_base = (uint8_t*)0x10000000;
  char *p = _uart_base;


  printf("%s %d\n", "printf test", 12345);
  
  for (int i = 0; str[i]; i++)
    _putchar(str[i]);
  
  putlong(p, (long)p);
  _putchar('\n');

  _puts("hartid ");
  putlong(p, hartid);
  _putchar('\n');
  
  _puts("ftd_base ");
  putlong(p, (long)ftd_base);
  _putchar('\n');


  _puts("_boot_a0 ");
  putlong(p, _boot_a0);
  _putchar('\n');
  
  _puts("_boot_a1 ");
  putlong(p, _boot_a1);
  _putchar('\n');

  fdt32_t *fdt = (fdt32_t*)_boot_a1;
  fdt32_t magic = fdt32_ld(fdt);
  _puts("magic ");
  putlong(p, magic);
  _putchar('\n');
}
