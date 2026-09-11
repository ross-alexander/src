#include <errno.h>
#include <string.h>

#include <cairomm/cairomm.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>

#include "common.h"

#define PK_version 89

#define PK_xxx1 240
#define PK_yyy 244
#define PK_post 245
#define PK_pre 247

/* ----------------------------------------------------------------------
--
--
--
---------------------------------------------------------------------- */
int PkGetNyb(unsigned char *packed, int *offset)
{
  int v;
  v = *offset % 2 ? packed[*offset >> 1] & 0x0f : packed[*offset >> 1] >> 4;
  (*offset)++;
  return v;
}

struct PkUnpackOut {
  int repeat;
  int length;
};

struct PkUnpackOut PkUnpack(unsigned char *packed, int dynf, int *offset)
{
  int i, j;
  struct PkUnpackOut out;
  out.repeat = 0;
  i = PkGetNyb(packed, offset);
  if (i == 0)
    {
      do {
	j = PkGetNyb(packed, offset);
	i++;
      } while (j == 0);

      while (i > 0)
	{
	  j = (j << 4) + PkGetNyb(packed, offset);
	  i--;
	}
      out.length = j - 15 + (13 - dynf)*16 + dynf;
      //      printf("l = %d\n", out.length);
    }
  else if (i <= dynf)
    out.length = i;
  else if (i < 14)
    out.length = (i - dynf - 1) * 16 + PkGetNyb(packed, offset) + dynf + 1;
  else if (i == 14)
    {
      out.repeat = PkUnpack(packed, dynf, offset).length;
      out.length = PkUnpack(packed, dynf, offset).length;
    }
  else
    {
      out.repeat = 1;
      out.length = PkUnpack(packed, dynf, offset).length;
    }
  return out;
}

/* ----------------------------------------------------------------------
--
-- PkLoadFile
--
---------------------------------------------------------------------- */

int DviFont::PkLoad(unsigned char *filename)
{
  FILE *fstream;
  int cmd, version, comment_len, ds, cs, hppp, vppp, offset, i, j;
  unsigned char *comment;

  if ((fstream = fopen((const char*)filename, "rb")) == NULL)
    {
      fprintf(stderr, "Error opening %s: %s\n", filename, strerror(errno));
      return 0;
    }

  DviStream* stream = new DviStream(fstream);

  if ((cmd = stream->u8()) != PK_pre)
    {
      fprintf(stderr, "%s: Does not look like a PK file.\n", filename);
      fclose(fstream);
      return 0;
    }
  if ((version = stream->u8()) != PK_version)
    {
      fprintf(stderr, "%s: PK file has incorrect version (%d != %d).\n", filename, version, PK_version);
      fclose(fstream);
      return 0;
    }
  comment_len = stream->u8();
  comment = stream->String(comment_len);

  if (debug)
    printf("%s\n", comment);

  /* --------------------
     ds - design size
     cs - checksum
     hppp - horizontal PPI
     vppp - vertical PPI
     -------------------- */

  ds = stream->u32();
  cs = stream->u32();
  hppp = stream->u32();
  vppp = stream->u32();
  long tell;

  if (debug)
    printf("PKFont: %d %d %d %d\n", ds, cs, hppp, vppp);

  while ((tell = stream->tell()) && ((cmd = stream->u8()) != PK_post))
    {
      if (debug)
	printf("%ld %d:\n", tell, cmd);
      if (cmd < PK_xxx1)
	{
	  int packet_length, end_of_packet;
	  int car;
	  int tfm_width, dx, dy, width, height, x_off, y_off;
	  
	  int dynf = cmd >> 4;
	  int flagbyte = cmd & 0x0f;
	  
	  /* --------------------
	     turnon - Start with black or white pixels
	     -------------------- */
	  
	  int turnon = (flagbyte & 0x08) >> 3;
	  
	  flagbyte &= 0x07;
	  if (flagbyte == 0x07)
	    {
	      packet_length = stream->u32();
	      car = stream->u32();
	      end_of_packet = stream->tell() + packet_length;

	      // printf("%ld: Flag byte = %d  Character = %d  Packet length = %d\n", tell, cmd, car, packet_length + 9);

	      tfm_width = stream->u32();
	      dx = stream->u32();
	      dy = stream->u32();
	      width = stream->u32();
	      height = stream->u32();
	      x_off = (char)stream->u32();
	      y_off = (char)stream->u32();

	      packet_length -= 28;
	    }
	  else if (flagbyte >= 0x03)
	    {
	      fprintf(stderr, "Extended format not implemented.");
	      exit(1);
	    }
	  else
	    {
	      /* --------------------
		 short format - bottom two bits of flag become top two bits of packet_length
		 -------------------- */
	      
	      packet_length = (flagbyte << 8) + stream->u8();
	      car = stream->u8();
	      end_of_packet = stream->tell() + packet_length;

	      // printf("%ld:  Flag byte = %d  Character = %d  Packet length = %d\n", tell, cmd, car, packet_length + 3);

	      tfm_width = stream->u24();
	      dx = stream->u8() << 16;
	      dy = 0;
	      width = stream->u8();
	      height = stream->u8();
	      x_off = (char)stream->u8();
	      y_off = (char)stream->u8();

	  /* --------------------
	     subtract off remaining header bytes off the packet length
	     -------------------- */
	      packet_length -= 8;
	    }
	  

	  if (debug)
	    {
	      printf("Character %d: %d %d\n", car, width, height);
	      printf("Flags: %d %d %d %d %d\n", dynf, turnon, flagbyte, packet_length, car);
	      printf("Metrics: %d %d %d (%d x %d) %d %d\n", tfm_width, dx, dy, width, height, x_off, y_off);
	    }
	  
	  unsigned char *packed = stream->String(packet_length);
	  
	  if (debug >= 2)
	    {
	      for (i = 0; i < packet_length; i++)
		{
		  if (i%8 == 0)
		    printf("%03d : ", i*2);
		  printf("%1x %1x ", packed[i] >> 4, packed[i] & 0x0F);
		  if ((i+1)%8 == 0)
		    printf("\n");
		}
	      printf("\n");
	    }
	  
	  unsigned char bitmap[width][height];
	  for (i = 0; i < width; i++)
	    for (j = 0; j < height; j++)
	      bitmap[i][j] = 0;
	  
	  if (dynf == 14)
	    {
	      unsigned char bitmap[width][height];
	      int byte_offset = 0;
	      int bit_offset = 0;
	      for (i = 0; i < width; i++)
		for (j = 0; j < height; j++)
		  {
		    int k = packed[byte_offset] & (1 << (7 - bit_offset));
		    bitmap[i][j] = k ? 1 : 0;
		    bit_offset = (bit_offset + 1) % 8;
		  }	      
	    }
	  else
	    {
	      int x = 0, y = 0, repeat = 0;
	      for (offset = 0; offset < 2 * packet_length && (y < height); )
		{
		  struct PkUnpackOut res;
		  res = PkUnpack(packed, dynf, &offset);
		  if (res.repeat)
		    {
		      if (debug)
			printf("[%d]", res.repeat);
		      repeat = res.repeat;
		    }
		  if (debug)
		    {
		      if (turnon)
			printf("%d", res.length);
		      else
			printf("(%d)", res.length);
		    }
		  for (int k = 0; k < res.length; k++)
		    {
		      bitmap[x++][y] = turnon;
		      if (x == width)
			{
			  x = 0;
			  y++;
			  if (repeat)
			    {
			      for (int j = 0; j < repeat; j++)
				{
				  for (int k = 0; k < width; k++)
				    bitmap[k][y] = bitmap[k][y-1];
				  y++;
				}
			      repeat = 0;
			    }
			}
		    }
		  turnon = 1 - turnon;
		}
	      if (debug)
		printf("\n");
	      if (debug)
		{
		  for (int y = 0; y < height; y++)
		    {
		      for (int x = 0; x < width; x++)
			printf("%c", bitmap[x][y] ? '*' : '.');
		      printf("\n");
		    }
		  printf("\n");
		}
	    }

	  int stride = Cairo::ImageSurface::format_stride_for_width(Cairo::Surface::Format::A8, width);
	  unsigned char *data = new unsigned char[height * stride];
	  for (int j = 0; j < height * stride; j++)
	    data[j] = 0;

	  for (int j = 0; j < height; j++)
	    for (int i = 0; i < width; i++)
	      data[j * stride + i] = bitmap[i][j] * 255;
	  
	  glyph[car].surface = Cairo::ImageSurface::create(data, Cairo::Surface::Format::A8, width, height, stride);
	  glyph[car].flags = 1;
	  glyph[car].w = width;
	  glyph[car].h = height;
	  glyph[car].tfm = tfm_width;
	  glyph[car].dx = dx;
	  glyph[car].dy = dy;
	  glyph[car].x_offset = x_off;
	  glyph[car].y_offset = y_off;
	  glyph[car].cc = car;
	  free(packed);
	}
      else if (cmd == PK_xxx1)
	{
	  int k = stream->u8();
	  unsigned char *special = stream->String(k);
	  free(special);
	}
      else if (cmd == PK_yyy)
	{
	  int yyy = stream->u32();
	}
      else
	{
	  printf("Unimplemented command %d.\n", cmd);
	  exit(1);
	}
    }
  return 1;
}
