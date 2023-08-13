#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <assert.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

/* ----------------------------------------------------------------------
--
-- gdkscale
--
-- Use Gdk-Pixbuf to scale images
--
-- 2022-02-18:
--   This is deprecated in favour of scale
--
-- 2021-08-17: Ross Alexnader
--   Change from HYPER to BILINEAR
--
---------------------------------------------------------------------- */



/* ----------------------------------------------------------------------
--
-- format_filename
--
---------------------------------------------------------------------- */

char *format_filename(const char *path, const char* suffix, int w, int h)
{
  regex_t rx;
  if (regcomp(&rx, "(\\.[A-Za-z]+)$", REG_EXTENDED))
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }
  regmatch_t match[2];

  if (regexec(&rx, path, 2, match, 0) == REG_NOMATCH)
    {
      fprintf(stderr, "regexec failure: %s\n", strerror(errno));
      exit(1);
    }

  #ifdef Debug
  for(int i = 0; i < 2; i++)
    {
      char buf[1024];
      fprintf(stdout, "%d %d\n", match[i].rm_so, match[i].rm_eo);
      strncpy(buf, path + match[i].rm_so, match[i].rm_eo - match[i].rm_so);
      buf[match[i].rm_eo - match[i].rm_so] = '\0';
      fprintf(stdout, "%s\n", buf);
    }
  #endif
  
  char buf[1024];
  strncpy(buf, path, match[0].rm_so);
  buf[match[0].rm_so] = '\0';
  
  snprintf(buf + match[0].rm_so, 1000, "_%d_%d.%s", w, h, suffix);
  return strdup(buf);
}

/* ----------------------------------------------------------------------
--
-- scale
--
---------------------------------------------------------------------- */
  
int scale(const char* file, int size, const char *format)
{
  GError *error = 0;

  /* Get pixbuf */
  
  GdkPixbuf *px = gdk_pixbuf_new_from_file(file, &error);
  assert(px != 0);
  
  int width = gdk_pixbuf_get_width(px);
  int height = gdk_pixbuf_get_height(px);
  int alpha = gdk_pixbuf_get_has_alpha(px);
  int depth = gdk_pixbuf_get_bits_per_sample(px);
  GdkColorspace color = gdk_pixbuf_get_colorspace(px);

  int width_new, height_new;

  double ratio = (double)width / height;
  double scale;
  if (ratio > 1.0)
    {
      width_new = size;
      height_new = size / ratio;
      scale = (double)size / width;
    }
  else
    {
      width_new = size * ratio;
      height_new = size;
      scale = (double)size / height;
    }

  char* nfile = format_filename(file, format, width_new, height_new);
  printf("%s: %d x %d -> %d x %d\n", file, width, height, width_new, height_new);

  GdkPixbuf *spx = gdk_pixbuf_new(color, alpha, depth, width_new, height_new);
  gdk_pixbuf_fill(spx, 0xffffffff);
  gdk_pixbuf_scale(px, spx, 0, 0, width_new, height_new, 0.0, 0.0, scale, scale, GDK_INTERP_BILINEAR);
  
  if (strcmp(format, "png") == 0)
    gdk_pixbuf_save(spx, nfile, format, &error, "compression", "9", NULL);
  else if (strcmp(format, "jpeg") == 0)
    gdk_pixbuf_save(spx, nfile, format, &error, "quality", "90", NULL);
  if (error)
    printf("%s: %s\n", nfile, error->message);
  free(nfile);
  return 1;
  }
    
/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  int size = 1000;
  const char* format = "png";
  int ch;
  
  while ((ch = getopt(argc, argv, "s:f:")) != EOF)
    switch(ch)
      {
      case 's':
	size = strtol(optarg, 0, 10);
	break;
      case 'f':
 	format = optarg;
	break;
      }
  
  if ((argc - optind) < 1)
    {
      fprintf(stdout, "%s [filename]\n", argv[0]);
      exit(0);
    }

  for (int i = optind; i < argc; i++)
    scale(argv[i], size, format);
}
