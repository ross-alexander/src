#include <assert.h>
#include <stdio.h>
#include <lua.hpp>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#include <algorithm>
#include <vector>
#include <string>
#include <random>
#include <ctime>

struct scanner_t {
  std::vector<const char*> dir;
  std::vector<std::string> file;
  int index;
};


/* ----------------------------------------------------------------------
--
-- add
--
---------------------------------------------------------------------- */

int add(lua_State *L)
{
  if (lua_gettop(L) == 0) return 0;
  if (!lua_isuserdata(L, 1)) return 0;

  scanner_t **scannerp = (scanner_t**)lua_touserdata(L, 1);
  scanner_t* scanner = *scannerp;

    /* --------------------
     Allow either strings or tables of strings to be added
     -------------------- */
  
  for (int i = lua_gettop(L); i > 1; i--)
    {
      if (lua_isstring(L, i))
	{
	  const char *s = lua_tostring(L, i);
	  scanner->dir.push_back(s);
	}
      if (lua_istable(L, i))
	{
	  /* table is in the stack at index 't' */
	  lua_pushnil(L);  /* first key */
	  while (lua_next(L, i) != 0)
	    {
	      if (lua_isstring(L, -1)) scanner->dir.push_back(lua_tostring(L, -1));
	      /* uses 'key' (at index -2) and 'value' (at index -1) */
	      /* removes 'value'; keeps 'key' for next iteration */
	      lua_pop(L, 1);
	    }
	  lua_pop(L, 1);
	}
      lua_pop(L, 1);
    }
  return 0;
}

/* ----------------------------------------------------------------------
--
-- Scanner_new
--
---------------------------------------------------------------------- */

int Scanner_new(lua_State *L)
{
  scanner_t** scanner = (scanner_t**)lua_newuserdata(L, sizeof(scanner_t*));
  *scanner = new scanner_t;
  luaL_getmetatable(L, "Scanner");
  lua_setmetatable(L, -2);

  /* Move newuserdata to bottem of the stack, rolling it was we go */
  
  lua_insert(L, 1);

  /* Add any paramters passed */

  add(L);

  /* Add some entropy */

  time_t  t = time(0);
  srand(t);
  return 1;
}

/* ----------------------------------------------------------------------
--
-- random
--
---------------------------------------------------------------------- */

int random(lua_State *L)
{
  if ((lua_gettop(L) == 0) || !lua_isuserdata(L, 1)) return 0;

  scanner_t *scanner = *((scanner_t**)lua_touserdata(L, 1));
  unsigned int s = scanner->file.size();
  if (s == 0)
    {
      lua_pushnil(L);
      return 1;
    }
  lua_pushstring(L, scanner->file[scanner->index].c_str());
  scanner->index = (scanner->index+1) % scanner->file.size();
  return 1;
}

/* ----------------------------------------------------------------------
--
-- files
--
---------------------------------------------------------------------- */

int files_get(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isuserdata(L, 1));

  scanner_t *scanner = *(scanner_t**)lua_touserdata(L, 1);
  lua_newtable(L);

  int index = 1;
  
  for(auto const& value: scanner->file)
    {  
      lua_pushstring(L, value.c_str());
      lua_rawseti(L, -2, index++);
    }
  return 1;
}

/* ----------------------------------------------------------------------
--
-- dirs_get
--
---------------------------------------------------------------------- */

int dirs_get(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isuserdata(L, 1));

  scanner_t *scanner = *(scanner_t**)lua_touserdata(L, 1);
  lua_newtable(L);

  int index = 1;
  
  for(auto const& value: scanner->dir)
    {  
      lua_pushstring(L, value);
      lua_rawseti(L, -2, index++);
    }
  return 1;
}
  


/* ----------------------------------------------------------------------
--
-- rescan
--
---------------------------------------------------------------------- */

static void rescan_dir(scanner_t *s, const char *dirpath)
{
  DIR *dirp;
  unsigned flen = pathconf(dirpath, _PC_NAME_MAX) + 1;
  unsigned dlen = offsetof(struct dirent, d_name) + flen;
  char *path = (char*)calloc(flen, 1);
  struct dirent **namelist;
  struct stat statbuf;
  int dcount;

  if ((dcount = scandir(dirpath, &namelist, 0, alphasort)) > 0)
    {
      for (int i = 0; i < dcount; i++)
	{
	  snprintf(path, flen, "%s/%s", dirpath, namelist[i]->d_name);
	  if (stat(path, &statbuf) == 0)
	    {
	      if (S_ISREG(statbuf.st_mode))
		{
		  s->file.push_back(std::string(path));
		}
	      if (S_ISDIR(statbuf.st_mode) && (strcmp(".", namelist[i]->d_name)) && (strcmp("..", namelist[i]->d_name)))
		{
		  rescan_dir(s, path);
		}
	    }
	}
      free(namelist);
    }
  free(path);
} 


int rescan(lua_State *L)
{
  if (lua_gettop(L) == 0) return 0;
  if (!lua_isuserdata(L, 1)) return 0;

  scanner_t **scannerp = (scanner_t**)lua_touserdata(L, 1);
  scanner_t* scanner = *scannerp;
  
  lua_newtable(L);
  int res = lua_gettop(L);

  /* --------------------
     Clear any existing list
     -------------------- */
  
  scanner->file.clear();
  
  for (unsigned i = 0; i < scanner->dir.size(); i++)
    {
      const char *dirpath = scanner->dir[i];
      rescan_dir(scanner, dirpath);
    }

  std::random_device rd;
  std::mt19937 g(rd());
  
  std::shuffle(scanner->file.begin(), scanner->file.end(), g);
  scanner->index = 0;
  return 1;
}

/* ----------------------------------------------------------------------
--
-- __tostring
--
---------------------------------------------------------------------- */

int Scanner___tostring(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isuserdata(L, 1));

  scanner_t **scannerp = (scanner_t**)lua_touserdata(L, 1);
  scanner_t* scanner = *scannerp;
  unsigned int len = 0;
  for (int i = 0; i < scanner->dir.size(); i++)
    {
      len += strlen(scanner->dir[i]) + 2;
    }
  char buf[1024 + len];

  snprintf(buf, 1024 + len, "Scanner(%p)", lua_touserdata(L, 1));
  unsigned int index = strlen(buf);
  if (len)
    {
      snprintf(buf + index, 1024 + len - index, " [");
      index = strlen(buf);
      for (int i = 0; i < scanner->dir.size(); i++)
	{
	  snprintf(buf + index, 1024 + len - index, "%s", scanner->dir[i]);
	  index = strlen(buf);
	  if (i < scanner->dir.size()-1)
	    {
	      snprintf(buf + index, 1024 + len - index, ", ");
	      index += 2;
	    }
	}
      snprintf(buf + index, 1024 + len - index, "]");
    }
  lua_pushstring(L, buf);
  return 1;
}

int scanner_index(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isuserdata(L, 1));

  //  printf("++ %d ++ %s\n", lua_gettop(L), lua_typename(L, lua_type(L, 1)));
  //  printf("++ %d ++ %s\n", lua_gettop(L), lua_typename(L, lua_type(L, 2)));

  scanner_t *scanner = *(scanner_t**)lua_touserdata(L, 1);
  const char* key = luaL_checkstring(L, 2);

  lua_pushvalue(L, lua_upvalueindex(1));
  lua_getfield(L, -1, key);
  if(!lua_isnil(L, -1))
    return 1;

  lua_pushvalue(L, lua_upvalueindex(2));
  lua_getfield(L, -1, key);

  //  printf("++ %d ++ %s\n", lua_gettop(L), lua_typename(L, lua_type(L, lua_gettop(L))));
  
  if (!lua_isnil(L, -1))
    {
      lua_pushvalue(L, 1);
      lua_call(L, 1, 1);
    }
  
  return 1;
}


extern "C" int luaopen_Scanner(lua_State *L)
{
  /* --------------------
     Create Scanner metatable
     -------------------- */

  lua_newtable(L);
  lua_pushcclosure(L, Scanner___tostring, 0);  lua_setfield(L, -2, "__tostring");

  /* --------------------
     methods table
     -------------------- */

  lua_newtable(L);
  lua_pushcclosure(L, add, 0);  lua_setfield(L, -2, "add");
  lua_pushcclosure(L, rescan, 0);  lua_setfield(L, -2, "rescan");
  lua_pushcclosure(L, random, 0);  lua_setfield(L, -2, "random");

  lua_newtable(L);
  lua_pushcclosure(L, files_get, 0); lua_setfield(L, -2, "files");
  lua_pushcclosure(L, dirs_get, 0); lua_setfield(L, -2, "dirs");

  lua_pushcclosure(L, scanner_index, 2); lua_setfield(L, -2, "__index");

  lua_setfield(L, LUA_REGISTRYINDEX, "Scanner");
  
  /* --------------------
     Create Scanner object
     -------------------- */

  lua_newtable(L);
  lua_pushcclosure(L, Scanner_new, 0);
  lua_setfield(L, -2, "new");
  return 1;
}

