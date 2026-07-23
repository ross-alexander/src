#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <regex.h>

#include "path_split.h"

#define PATH_SPLIT_DEBUG 0

/* ----------------------------------------------------------------------
   --
   -- math_path_rx
   --
   ---------------------------------------------------------------------- */

struct path_part_t* match_path_rx(const char *path)
{
  regex_t rx;
  if (regcomp(&rx, "(/?)([^/]*)", REG_EXTENDED))
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }

  regmatch_t match[3];

  int start = 0;
  int length = strlen(path);
  int match_count = 0;

  /* Loop over path, splitting before trailing / */
  /* Get count before creating offset array */
  
  while(start < length)
    {
      if (regexec(&rx, path + start, 3, match, 0) == REG_NOMATCH)
	{
	  fprintf(stderr, "regexec failure: %s\n", strerror(errno));
	  exit(1);
	}
      start += match[0].rm_eo;
      match_count++;
    }

  /* Allocate array with sentinal at the end */
  
  struct path_part_t *components = calloc(match_count + 1, sizeof(struct path_part_t));
  int count = 0;
  start = 0;

  /* Loop over path, splitting before trailing / */

  while(start < length)
    {
      regexec(&rx, path + start, 3, match, 0);
      components[count].start = start + match[0].rm_so;
      components[count].length = match[0].rm_eo - match[0].rm_so;
      components[count].slash = (match[1].rm_eo - match[1].rm_so) > 0 ? 1 : 0;
      start += match[0].rm_eo;
      count++;
    }

  /* Set sentinal */
  
  components[count].start = -1;
  return components;
}

/* ----------------------------------------------------------------------
   --
   -- path_split
   --
   ---------------------------------------------------------------------- */

struct path_ext_t* path_split(const char *path)
{
  struct path_part_t *parts = match_path_rx(path);

  int count;

  /* count parts, with sentinal (start = -1) at the end */
  
  for (count = 0; parts[count].start >= 0; count++)
#if PATH_SPLIT_DEBUG
    printf("%02d: %.*s %c\n", count, parts[count].length, path + parts[count].start, parts[count].slash ? '/' : ' ');
#else
    ;
#endif
  
  int file_length = parts[count-1].length;

  /* Allow result struct on stack */
  
  struct path_ext_t *split = calloc(sizeof(struct path_ext_t), 1);
  
  /* Sum directory components */
  
  int dir_length = 0;
  for (int i = 0; i < (count-1); i++)
    dir_length += parts[i].length;

  /* If directory components reconstruct into string without trailing / */

  if (dir_length)
    {
      split->dir = calloc(sizeof(char), dir_length + 1);
      int offset = 0;
      for (int i = 0; i < (count-1); i++)
	{
	  strncpy(split->dir + offset, path + parts[i].start, parts[i].length);
	  offset += parts[i].length;
	}
    }

  /* If file try to extract extension */
  
  if (file_length)
    {
      regex_t rx;
      if (regcomp(&rx, "(^[/])?(.*)\\.([A-Za-z]+)$", REG_EXTENDED))
	{
	  fprintf(stderr, strerror(errno));
	  exit(1);
	}
      regmatch_t match[4];

      /* Match extension */
      
      if (regexec(&rx, path + parts[count-1].start, 4, match, 0) != REG_NOMATCH)
	{
	  int file_no_ext_length = match[2].rm_eo - match[2].rm_so;
	  int ext_length = match[3].rm_eo - match[3].rm_so;

	  /* if file + extension */
	  
	  if (file_no_ext_length > 0)
	    {
	      split->file = calloc(sizeof(char), file_no_ext_length + 1);
	      strncpy(split->file, path + parts[count-1].start + match[2].rm_so, file_no_ext_length);
	    }

	  /* Extension, which will always have at least one character */

	  split->ext = calloc(sizeof(char), ext_length + 1);
	  strncpy(split->ext, path + parts[count-1].start + match[3].rm_so, ext_length);
	}
      else
	{
	  /* Check if leading / and remove from filename */
	  regex_t rx;
	  if (regcomp(&rx, "(^[/])?(.*)", REG_EXTENDED))
	    {
	      fprintf(stderr, strerror(errno));
	      exit(1);
	    }
	  regmatch_t match[3];

	  /* Should always match */
	  
	  regexec(&rx, path + parts[count-1].start, 3, match, 0);
	  
	  split->file = calloc(sizeof(char), parts[count-1].length + 1);
	  strncpy(split->file, path + parts[count-1].start + match[2].rm_so, match[2].rm_eo - match[2].rm_so);
	}
    }

  /* Case with directory is the root (/) only */
  
  if (!split->dir && parts[count-1].slash)
      split->dir = strdup("/");
    free(parts);

#if PATH_SPLIT_DEBUG
  if (split->dir) printf("dir: %s\n", split->dir);
  if (split->file) printf("file: %s\n", split->file);
  if (split->ext) printf("ext: %s\n", split->ext);
#endif
  
  return split;
}
