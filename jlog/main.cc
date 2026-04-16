/* ----------------------------------------------------------------------
--
-- main.cc
--
---------------------------------------------------------------------- */
   
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
-- Currently doesn't not handle TZ
--
---------------------------------------------------------------------- */

double tstod(const char *s)
{
  size_t date_start = 0;
  size_t date_len = strcspn(s, "T");
  size_t time_start = date_start + date_len + 1;
  size_t time_len = strcspn(s + time_start, ".");
  size_t deci_start = time_start + time_len;
  size_t deci_len = strcspn(s + deci_start, "+Z");
  size_t zone_start = deci_start + deci_len;
  size_t zone_len = strlen(s) - zone_start;
  
  char *zeit = (char*)alloca(strlen(s) + 1);
  memset(zeit, 0, strlen(s));
  strncpy(zeit, s, date_len);
  zeit[date_len] = ' ';
  strncpy(zeit + date_len + 1, s + time_start, time_len);
  
  zeit[date_len + time_len + 1] = ' ';
  strncpy(zeit + date_len + time_len + 2, s + zone_start, zone_len);

  char *deci = (char*)alloca(deci_len + 3);
  strncpy(deci, s + deci_start, deci_len);
  deci[deci_len] = '\0';
  struct tm tm;
  strptime(zeit, "%Y-%m-%d %H:%M:%S", &tm);
  time_t epoch = timegm(&tm);

  double res = (double)epoch + strtod(deci, nullptr);
  return res;
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

  // Disable IP to Name resolving
  
  lua_pushinteger(L, 0);
  lua_setglobal(L, "resolve");

  collector_t *cl = new collector_t(L);

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

