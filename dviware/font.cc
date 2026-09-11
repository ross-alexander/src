#include <assert.h>
#include <stdio.h>

#include <cairomm/cairomm.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <gtk/gtk.h>

namespace kpse {
#define NO_DEBUG 1
  extern "C" {
#include <kpathsea/kpathsea.h>
  }
}

#include "common.h"

/* ----------------------------------------------------------------------
--
-- DviFontLoad
--
---------------------------------------------------------------------- */
DviFont::DviFont(DviStream *stream)
{
  int alen, llen;
  debug = 0;
  id = stream->u8();
  checksum = stream->u32();
  scaled_size = stream->u32();
  design_size = stream->u32();
  alen = stream->u8();
  llen = stream->u8();
  name = stream->String(alen + llen);
  printf("%s %d %d\n", name, scaled_size, design_size);
  glyph = new DviGlyph[256];
  for (int k = 0; k < 256; k++)
    glyph[k].font = this;
}

int DviFont::Load(int r)
{
  res = r;

  kpse::kpse_glyph_file_type type;
  unsigned char *tfmfile = (unsigned char*)kpse::kpse_find_file((char*)name, kpse::kpse_tfm_format, true);
  unsigned char *glyphfile = (unsigned char*)kpse::kpse_find_glyph((char*)name, res, kpse::kpse_any_glyph_format, &type);
  if (tfmfile) printf("%s\n", tfmfile);

  if (!glyphfile)
    {
      fprintf(stderr, "Could not find %s @ %d\n", name, r);
    }
  
  assert(glyphfile);

  if (glyphfile) printf("%s\n", glyphfile);
  switch (type.format)
    {
    case kpse::kpse_pk_format:
      PkLoad(glyphfile);
      break;
      ;;
    default:
      ;;
    }
  return 1;
}
