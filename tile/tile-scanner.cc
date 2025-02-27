#include <assert.h>
#include <stdio.h>
#include <lua.hpp>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <random>
#include <filesystem>

struct scanner_t {
  std::vector<std::filesystem::path> dir;
  std::vector<std::filesystem::path> file;
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
-- scanner_t_new
--
---------------------------------------------------------------------- */

int scanner_t_new(lua_State *L)
{
  scanner_t** scanner = (scanner_t**)lua_newuserdata(L, sizeof(scanner_t*));
  *scanner = new scanner_t;
  //  lua_getfield(L, LUA_REGISTRYINDEX, "scanner_t");
  luaL_getmetatable(L, "tile_scanner_t");
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
      lua_pushstring(L, value.c_str());
      lua_rawseti(L, -2, index++);
    }
  return 1;
}
  


/* ----------------------------------------------------------------------
--
-- rescan
--
---------------------------------------------------------------------- */

static void rescan_dir(scanner_t *s, std::filesystem::path &dirpath)
{
  if (is_directory(dirpath))
    {
      
      for (auto const& dir_entry : std::filesystem::directory_iterator{dirpath})
	{
	  std::filesystem::path p = dir_entry.path();
	  if (is_regular_file(p))
	    {
	      s->file.push_back(p);
	    }
	  if (is_directory(p))
	    {
	      rescan_dir(s, p);
	    }
	}      
    }
} 


int rescan(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  assert(luaL_checkudata(L, 1, "tile_scanner_t"));

  scanner_t *scanner = *(scanner_t**)lua_touserdata(L, 1);

  lua_newtable(L);
  int res = lua_gettop(L);

  /* --------------------
     Clear any existing list
     -------------------- */
  
  scanner->file.clear();
  
  for (unsigned i = 0; i < scanner->dir.size(); i++)
    {
      rescan_dir(scanner, scanner->dir[i]);
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

int scanner_t___tostring(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isuserdata(L, 1));

  scanner_t **scannerp = (scanner_t**)lua_touserdata(L, 1);
  scanner_t* scanner = *scannerp;
  unsigned int len = 0;
  for (int i = 0; i < scanner->dir.size(); i++)
    {
      len += strlen(scanner->dir[i].c_str()) + 2;
    }
  char buf[1024 + len];

  snprintf(buf, 1024 + len, "scanner_t(%p)", lua_touserdata(L, 1));
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

extern "C" int luaopen_scanner_t(lua_State *L)
{
  /* --------------------
     Create scanner_t metatable
     -------------------- */

  lua_newtable(L);
  lua_pushcclosure(L, scanner_t___tostring, 0);  lua_setfield(L, -2, "__tostring");

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

  lua_setfield(L, LUA_REGISTRYINDEX, "tile_scanner_t");
  
  /* --------------------
     Create scanner_t object
     -------------------- */

  lua_pushglobaltable(L);
  lua_newtable(L);
  lua_pushcclosure(L, scanner_t_new, 0);
  lua_setfield(L, -2, "new");
  lua_setfield(L, -2, "scanner_t");
  return 1;
}
