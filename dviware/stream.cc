#include <stdio.h>

#include <cairomm/cairomm.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>

#include "common.h"

/* ----------------------------------------------------------------------
--
--
--
---------------------------------------------------------------------- */

DviStream::DviStream(FILE *f)
{
  stream = f;
}

unsigned int DviStream::u8()
{
  return fgetc(stream);
}

unsigned int DviStream::u16()
{
  return (DviStream::u8() << 8 | DviStream::u8());
}

unsigned int DviStream::u24()
{
  return DviStream::u8() << 16 | DviStream::u16();
}

unsigned int DviStream::u32()
{
  return (DviStream::u16() << 16 | DviStream::u16());
}

unsigned char *DviStream::String(int len)
{
  unsigned char *s = new unsigned char[len + 1];
  fread(s, 1, len, stream);
  s[len] = '\0';
  return s;
}

int DviStream::s8()
{
  unsigned int i = u8();
  if (i & 0x80) i = i | 0xffffff00;
  return i;
}

int DviStream::s16()
{
  unsigned int i = u16();
  if (i & 0x8000) i = i | 0xffff0000;
  return i;
}

int DviStream::s24()
{
  unsigned int i = u24();
  if (i & 0x800000) i = i | 0xff000000;
  return i;
}

int DviStream::s32()
{
  unsigned int i = u32();
  return i;
}

int DviStream::seek(signed long offset, int whence)
{
  return fseek(stream, offset, whence);
}

long DviStream::tell()
{
  return ftell(stream);
}
