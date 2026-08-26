#define PCRE2_CODE_UNIT_WIDTH 8

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <pcre2.h>

struct path_part_t {
  int32_t start, length, slash;
};

struct path_exp_t {
  char *dir;
  char *file;
  char *ext;
};

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

  struct path_part_t *components = calloc(match_count + 1, sizeof(struct path_part_t));
  int count = 0;
  start = 0;

  while(start < length)
    {
      regexec(&rx, path + start, 3, match, 0);
      components[count].start = start + match[0].rm_so;
      components[count].length = match[0].rm_eo - match[0].rm_so;
      components[count].slash = (match[1].rm_eo - match[1].rm_so) > 0 ? 1 : 0;
      start += match[0].rm_eo;
      count++;
    }

  components[count].start = -1;

  return components;
}

struct path_part_t* match_path_pcre2(const char *path)
{
  int match_count = 0;

  pcre2_code *re;
  PCRE2_SPTR pattern;
  PCRE2_SPTR subject;
  //  PCRE2_SPTR name_table;

  PCRE2_SIZE erroroffset;
  int errornumber;
  
  pattern = (PCRE2_SPTR)"(/?)([^/]*)";
  subject = (PCRE2_SPTR)path;

  size_t subject_length = strlen((char*)subject);

  re = pcre2_compile(pattern,
		     PCRE2_ZERO_TERMINATED,
		     0,
		     &errornumber,
		     &erroroffset,
		     nullptr);

  if (re == nullptr)
    {
      PCRE2_UCHAR buffer[256];
      pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
      printf("PCRE2 complication failed at offset %d: %s\n", (int)erroroffset, buffer);
      return nullptr;
    }
  
  pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, nullptr);

  int rc = pcre2_match(re, subject, subject_length, 0, 0, match_data, nullptr);

  if (rc < 0)
    {
      switch(rc)
	{
	case PCRE2_ERROR_NOMATCH:
	  printf("No match\n");
	  break;
	default:
	  printf("Matching error %d\n", rc);
	  break;
	}
      pcre2_match_data_free(match_data);
      pcre2_code_free(re);
      return nullptr;
    }
  PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
  
  for (;;)
    {
      uint32_t options = 0;
      PCRE2_SIZE start_offset = ovector[1];

      if (ovector[0] == ovector[1])
	break;
      
      match_count++;
      
      rc = pcre2_match(re, subject, subject_length, start_offset, options, match_data, nullptr);
      
      if(rc == PCRE2_ERROR_NOMATCH)
	break;
      if (rc < 0)
	{
	  printf("Matching error %d\n", rc);
	  pcre2_match_data_free(match_data);
	  pcre2_code_free(re);
	  return nullptr;
	}
    }

  struct path_part_t *components = calloc(match_count + 1, sizeof(struct path_part_t));
  int count = 0;

  rc = pcre2_match(re, subject, subject_length, 0, 0, match_data, nullptr);
  components[count].start = ovector[0];
  components[count].length = ovector[1] - ovector[0];
  components[count].slash = ovector[3] > 0 ? 1: 0;

  for (;;)
    {
      uint32_t options = 0;
      PCRE2_SIZE start_offset = ovector[1];

      if (ovector[0] == ovector[1])
	break;

      count++;

      rc = pcre2_match(re, subject, subject_length, start_offset, options, match_data, nullptr);
      
      if(rc == PCRE2_ERROR_NOMATCH)
	break;
      components[count].start = ovector[0];
      components[count].length = ovector[1] - ovector[0];
      components[count].slash = ovector[3] > 0 ? 1: 0;
    }

  components[count].start = -1;
  
  pcre2_match_data_free(match_data);
  pcre2_code_free(re);
  return components;
}

void path_split(const char *path)
{
  struct path_part_t *parts = match_path_rx(path);

  int count;
  for (count = 0; parts[count].start >= 0; count++)
    printf("%02d: %.*s %c\n", count, parts[count].length, path + parts[count].start, parts[count].slash ? '/' : ' ');

  int file_length = parts[count-1].length;
  int dir_length = 0;
  for (int i = 0; i < (count-1); i++)
    dir_length += parts[i].length;

  struct path_exp_t *split = calloc(sizeof(struct path_exp_t), 1);

  if (dir_length)
    {
      split->dir = calloc(sizeof(char), dir_length + 1);
      int offset = 0;
      for (int i = 0; i < (count-1); i++)
	{
	  strncpy(split->dir + offset, path + parts[i].start, parts[i].length);
	  offset += parts[i].length;
	}
      printf("** dir: %s\n", split->dir);
    }
  if (file_length)
    {
      regex_t rx;
      if (regcomp(&rx, "(^[/])?(.*)\\.([A-Za-z]+)$", REG_EXTENDED))
	{
	  fprintf(stderr, strerror(errno));
	  exit(1);
	}
      regmatch_t match[4];

      if (regexec(&rx, path + parts[count-1].start, 4, match, 0) != REG_NOMATCH)
	{
	  int file_no_ext_length = match[2].rm_eo - match[2].rm_so;
	  int ext_length = match[3].rm_eo - match[3].rm_so;

	  if (file_no_ext_length > 0)
	    {
	      split->file = calloc(sizeof(char), file_no_ext_length + 1);
	      strncpy(split->file, path + parts[count-1].start + match[2].rm_so, file_no_ext_length);
	      printf("** file: %s\n", split->file);
	    }

	  if (ext_length > 0)
	    {
	      split->ext = calloc(sizeof(char), ext_length + 1);
	      strncpy(split->ext, path + parts[count-1].start + match[3].rm_so, ext_length);
	      printf("** ext: %s\n", split->ext);
	    }
	}
    }
  printf("\n");
}

int main()
{
  const char* paths[] = {
    "just_file.jpg",
    "/locker/images/202607/test_image.jpg",
    "/silly/path/",
    "test/foo.jpg",
    nullptr,
  };

  for (int i = 0; paths[i]; i++)
    {
	  path_split(paths[i]);
    }
}

