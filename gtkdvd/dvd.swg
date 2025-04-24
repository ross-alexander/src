/* ----------------------------------------------------------------------
   --
   -- 2023-03-30: Add emacs local variables
   --
   ---------------------------------------------------------------------- */

%module dvd

%{
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/cdrom.h>
#include <unistd.h>

#include <dvdread/ifo_read.h>
#include "glib.h"
#include "common.h"

char *device;

dvd_info_t* dvd_read(const char *dev)
{
  const char *dvd_device = dev ? dev : device;
  printf("Using device %s\n", dvd_device);
  dvd_info_t *info = lsdvd_read_dvd(dvd_device);
  return info;
}

dvd_title_t *info_get_title(dvd_info_t *info, int i)
{
  dvd_title_t *title = &info->titles[i-1];
  title->dvd_info = info;
  return title;
}

int eject(const char *device)
{
  int fd;
  if ((fd = open(device,O_RDWR|O_NONBLOCK)) == -1)
    {
      printf("Error opening device: \"%s\", ",device);
      perror("");
      exit(1);
    }
  if (ioctl(fd, CDROM_LOCKDOOR, 0) != 0)
    {
      printf("Error unlocking device: \"%s\", ",device);
      perror("");
      exit(1);
    }
  ioctl(fd, CDROMEJECT, 1);
  close(fd);
}

/* ----------------------------------------------------------------------
--
-- info_read_title
--
---------------------------------------------------------------------- */
 
int info_read_title(dvd_info_t *info, int title, const char *file)
{
  printf("Device = %s\n", info->discinfo.device);
  int n_chapters = info->titles[title - 1].chapter_count_reported;

  FILE *stream;
  if ((stream = fopen(file, "wb")) == 0)
    return 0;

  unsigned char *readdata = malloc(1024 * DVD_VIDEO_LB_LEN);
  for (int j = 1; j <= n_chapters; j++)
    {
      tc_dvd_read(info->discinfo.device, title, j, 1, readdata, 1, stream);
      printf("Writing chapter %d of title %d to %s\n", j, title, file);
    }
  fclose(stream);
      
  //  printf("Writing title %d (%d chapters) to %s from %s\n", e[i].title, n_chapters, e[i].file, device);
  return 1;
}

/* ----------------------------------------------------------------------
--
-- dvd_info_table
--
---------------------------------------------------------------------- */

int dvd_info_table(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  dvd_info_t *info;
  assert(SWIG_IsOK(SWIG_ConvertPtr(L, 1, (void**)&info, SWIGTYPE_p_dvd_info_t, 0)));
  
  lua_newtable(L);
  lua_newtable(L);
  lua_pushstring(L, info->discinfo.device); lua_setfield(L, -2, "device");
  lua_pushstring(L, info->discinfo.disc_title); lua_setfield(L, -2, "disc_title");
  lua_pushstring(L, info->discinfo.vmg_id); lua_setfield(L, -2, "vmg_id");
  lua_pushstring(L, info->discinfo.provider_id); lua_setfield(L, -2, "provider_id");
  lua_setfield(L, -2, "discinfo");
  
   /* titles */

  lua_newtable(L);
  for (int i = 0; i < info->title_count; i++)
    {
      lua_newtable(L);
      dvd_title_t *t = &info->titles[i];
      lua_pushinteger(L, t->num);
      lua_setfield(L, -2, "num");
      
      /* general */
      lua_newtable(L);
      lua_pushnumber(L, t->general.length); lua_setfield(L, -2, "length");
      lua_setfield(L, -2, "general");
      
      /* video */
      
      lua_newtable(L);
      lua_pushnumber(L, t->parameter.fps); lua_setfield(L, -2, "fps");
      lua_pushstring(L, t->parameter.format); lua_setfield(L, -2, "format");
      lua_pushstring(L, t->parameter.aspect); lua_setfield(L, -2, "aspect");
      lua_pushinteger(L, strtol(t->parameter.width, 0, 10)); lua_setfield(L, -2, "width");
      lua_pushinteger(L, strtol(t->parameter.height, 0, 10)); lua_setfield(L, -2, "height");
      lua_setfield(L, -2, "parameter");
      
       /* audio tracks */
      
      lua_newtable(L);
      for (int j = 0; j < t->audiostream_count; j++)
	{
	  struct audiostream *a = &t->audiostreams[j];
	  lua_newtable(L);
	  
	  lua_pushstring(L, a->langcode); lua_setfield(L, -2, "langcode");
	  lua_pushstring(L, a->language); lua_setfield(L, -2, "language");
	  lua_pushstring(L, a->format); lua_setfield(L, -2, "format");
	  lua_pushnumber(L, strtod(a->frequency, 0)); lua_setfield(L, -2, "frequency");
	  lua_pushstring(L, a->quantization); lua_setfield(L, -2, "quantization");
	  lua_pushinteger(L, a->channels); lua_setfield(L, -2, "channels");
	  lua_pushinteger(L, a->streamid); lua_setfield(L, -2, "streamid");
	  
	  
	  lua_rawseti(L, -2, j+1);
	}
      lua_setfield(L, -2, "audiotracks");
      
      lua_rawseti(L, -2, i+1);
    }
  lua_setfield(L, -2, "titles");
  return 1;
}

  /* ----------------------------------------------------------------------
   --
   -- cli_options
   --
   ---------------------------------------------------------------------- */

int cli_options(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_istable(L, 1));
  int t = 1;
  
  /* table is in the stack at index 't' */
  lua_pushnil(L);  /* first key */
  while (lua_next(L, t) != 0)
    {
       /* uses 'key' (at index -2) and 'value' (at index -1) */
      printf("%s - %s\n",
	     lua_typename(L, lua_type(L, -2)),
	     lua_typename(L, lua_type(L, -1)));
      printf("%d - %s\n",  lua_tointeger(L, -2), lua_tostring(L, -1));
      /* removes 'value'; keeps 'key' for next iteration */
      lua_pop(L, 1);
    }  
  exit(1);
}


%}


%init {
  device = DVD_DEFAULT_DEVICE;
}

extern char *device;

/* ----------------------------------------------------------------------
--
-- class extentions
--
---------------------------------------------------------------------- */

%include "dvd.h"

%extend dvd_title_t {
  int copy(const char* vfile, const char* ifile)
  {
    int ret;

    if (vfile)
      {
	ret = info_read_title(self->dvd_info, self->num, vfile);
      }
    if (ifile)
      {
	char *s = title_print(self);
	FILE *stream;
	if (stream = fopen(ifile, "w"))
	  {
	    fprintf(stream, "%s", s);
	    fclose(stream);
	  }
	free(s);
      }
    return ret;
  }
  const char* to_string()
  {
    const char *s = title_print(self);
    return s;
  }
}

/* --------------------
dvd structure extentions
-------------------- */

%extend dvd_info_t {
  dvd_info_t(const char* s)
    {
      dvd_info_t *info = dvd_read(s);
      return info;
    };

  dvd_title_t* get_title(int t)
  {
    return info_get_title(self, t);
  };

  char *get_disc_name()
  {
    char *t = calloc(sizeof(char), 33);
    get_title_name(self->discinfo.device, t);
    return t;
  }
};


dvd_info_t* dvd_read(const char*);
dvd_title_t* info_get_title(dvd_info_t*, int);
int info_read_title(dvd_info_t*, int, const char*);
char* title_print(dvd_title_t*);
int eject(const char*);

%native(dvd_info_table) int dvd_info_table(lua_State *L);
%native(cli_options) int cli_options(lua_State *L);

/* Local Variables:  */
/* mode: c           */
/* comment-column: 0 */
/* End:              */
