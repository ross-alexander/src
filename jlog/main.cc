#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <alloca.h>

#include <string>
#include <map>
#include <iostream>

#include <jsoncons/json.hpp>
using jsoncons::json;

#include <lua.hpp>


#include "common.h"
#include "parse.tab.h"

/* ----------------------------------------------------------------------
--
-- tstod
--
-- Currently doesn't not handle TZ or decimal
--
---------------------------------------------------------------------- */

double tstod(const char *s)
{
  size_t len_date, len_time, len_deci;
  len_date = strcspn(s, "T");
  len_time = strcspn(s + len_date + 1, ".");
  len_deci = strcspn(s + len_date + len_time + 2, "+Z");

  char *zeit = (char*)alloca(len_date + len_time + 3);
  strncpy(zeit, s, len_date);
  zeit[len_date] = ' ';
  strncpy(zeit + len_date + 1, s + len_date + 1, len_time);
  zeit[len_date + len_time + 1] = '\0';

  struct tm tm;
  strptime(zeit, "%Y-%m-%d %H:%M:%S", &tm);
  time_t epoch = timegm(&tm);
  return epoch;
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

extern int yydebug;

int main(int argc, char * const argv[])
{
  const char *in = 0;
  const char *out = 0;
  const char *log = 0;
  const char *lua = 0;

  FILE *stream_in = stdin;
  FILE *stream_out = stdout;
  FILE *stream_log = 0;

  /* --------------------
     Create lua state
     -------------------- */

  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  int ch;
  while ((ch = getopt(argc, argv, "di:o:l:c:t:")) != EOF)
    switch(ch)
      {
      case 'i':
	printf("Using %s for input file.\n", optarg);
	stream_in = fopen(optarg, "r");
	assert(stream_in);
	break;
      case 'd':
	yydebug = 1;
	break;
      case 'o':
	out = optarg;
	break;
	
      case 'l':
	log = optarg;
	break;

      case 'c':
	lua = optarg;
	break;
      case 't':
	{
	  time_t epoch = time(0);
	  lua_pushinteger(L, epoch - (60 * strtol(optarg, 0, 10)));
	  lua_setglobal(L, "tdiff");
	}
	break;
      }

  if (lua == 0)
    {
      fprintf(stderr, "%s: requires -c [lua file] to run.\n", argv[0]);
      exit(1);
    }

  int ret;
  if ((ret = luaL_loadfile(L, lua)) == 0)
    ret = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (ret)
    {
      assert(lua_isstring(L, -1));
      const char *error = lua_tolstring(L, -1, 0);
      fprintf(stderr, "Error: %s\n", error);
      exit(1);
    }

  printf("Loaded lua file %s\n", lua);

  lua_pushinteger(L, 0);
  lua_setglobal(L, "resolve");

  collector *cl = new collector(L);

  yystart(stream_in);

  int stdout_old, fd;

  if (log)
    {
      fd = open(log, O_CREAT|O_WRONLY, 0644);
      printf("Using %s for logging.\n", log);
      assert(fd >= 0);
      stdout_old = dup(1);
      dup2(fd, 1);
    }

  yyparse(cl);

  if (log)
    {
      close(fd);
      dup2(stdout_old, 1);
    }

  if (out)
    {
      stream_out = freopen(out, "w", stdout);
      printf("Using %s for output.\n", out);
      cl->dump();
      fclose(stream_out);
    }
  else
    {
      cl->dump();
    }
}

