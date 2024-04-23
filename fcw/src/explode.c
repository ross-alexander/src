#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <stdint.h>

#include "fcw.h"
#include "huffman.h"

/* ----------------------------------------------------------------------
--
-- DecodeFile
--
---------------------------------------------------------------------- */

int DecodeFile(fcw_global_t *globals, char *file)
{
  size_t bsize = 0;
  uint8_t *decodebuf = NULL;

  struct stat statbuf;
  int res;
  if (stat(file, &statbuf) != 0)
    {
      fprintf(stderr, "Cannot stat %s: %s\n", file, strerror(errno));
      exit(1);
    }

  FILE *stream = fopen(file, "rb");
  if (stream == NULL)
    {
      fprintf(stderr, "Failed to open file %s: %s\n", file, strerror(errno));
      exit(1);
    }

  unsigned char header[128];
  fseek(stream, 0, SEEK_SET);
  fread(header, 128, 1, stream);


  size_t fsize = statbuf.st_size;
  unsigned char *inbuf = (unsigned char*)malloc(fsize - 128);
  fseek(stream, 128, SEEK_SET);
  fread(inbuf, fsize - 128, 1, stream);
  fclose(stream);
 
 if (header[48])
    {
      printf("Compressed\n");

      struct Huff *huff = HuffDecodeBuffer(fsize - 128, inbuf);
#ifdef WriteEnabled
      header[48] = 0;
      char *outfile = calloc(strlen(file) + 5, 1);
      strcpy(outfile, file);
      strcat(outfile, ".out");
      
      res = 1;
      if ((stream = fopen(outfile, "wb")) == NULL)
	{
	  fprintf(stderr, "Cannot open %s for writing: %s\n",
		  outfile, strerror(errno));
	  res = 0;
	}
      else
	{
	  fwrite(header, 128, 1, stream);
	  fwrite(huff->out, huff->outoffset, 1, stream);
	  fclose(stream);
	}
      free(outfile);
#endif
      bsize = huff->outoffset;
      decodebuf = (uint8_t*)calloc(1, bsize);
      memcpy(decodebuf, huff->out, bsize);
      free(huff->out);
      free(huff);
      free(inbuf);
    }
  else
    {
      printf("Uncompressed\n");
      bsize = fsize - 128;
      decodebuf = (uint8_t*)calloc(1, bsize);
      memcpy(decodebuf, inbuf, bsize);
      free(inbuf);
    }
 if (bsize && decodebuf)
   {
     res = FcwDecodeBuffer(globals, file, bsize, decodebuf);
     free(decodebuf);
   }
 return res;
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  int ch;

  fcw_global_t globals;
  
  while ((ch = getopt(argc, argv, "dc")) != EOF)
    switch(ch)
      {
      case 'd':
	globals.dump = 1;
	break;
      case 'c':
	globals.cairo = 1;
	break;
      }

  if (optind >= argc)
    {
      fprintf(stdout, "%s: [filename]\n", argv[0]);
      exit(0);
    }
  DecodeFile(&globals, argv[optind]);
  exit(0);
}
