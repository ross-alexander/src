/* ----------------------------------------------------------------------
--
-- iphacks
--
-- 20260316: Add function to sort table based on IP address
--
---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <lua.hpp>

#include <jsoncons/json.hpp>

/* ----------------------------------------------------------------------
   --
   -- sort table based on IP address
   --
   ---------------------------------------------------------------------- */

typedef struct keypair_t {
  uint32_t c_key;
  const char *lua_key;
} keypair_t;

int keypair_compare(const void *a, const void *b)
{
  keypair_t* kpa = (keypair_t*)a;
  keypair_t* kpb = (keypair_t*)b;
  return kpa->c_key < kpb->c_key ? -1 : (kpa->c_key > kpb->c_key ? 1 : 0);
}


int iphacks_ipkeysort(lua_State *L)
{
  /* Safety checks */
  
  if (lua_gettop(L) < 1)
    return 0;
  if (!lua_istable(L, 1))
      return 0;

  /* --------------------
     Loop over table to check it is valid and
     count number of entries
     -------------------- */
  
  int t = 1; /* table index */
  int valid = 1;
  unsigned int count = 0;

  lua_pushnil(L);  /* first key */
  while (lua_next(L, t) != 0)
    {
      /* uses 'key' (at index -2) and 'value' (at index -1) */
      struct in_addr a;
      if (inet_aton(luaL_checkstring(L, -2), &a) == 0)
	{
	  valid = 0;
	  break;
	}
      /* removes 'value'; keeps 'key' for next iteration */
      count++;
      lua_pop(L, 1);
    }

  /* return nil if not valid */
  
  if (!valid)
    {
      lua_pushnil(L);
      return 1;
    }

  /* Create array to be sorted */
  
  keypair_t *klist = new keypair_t[count];

  /* Reset count and fill klist array */
  
  count = 0;
  lua_pushnil(L);
  while (lua_next(L, t) != 0)
    {
      /* Convert IP address to (little endian) uint32_t */
      struct in_addr a;
      inet_aton(luaL_checkstring(L, -2), &a);
      klist[count].c_key = ntohl(a.s_addr);
      klist[count].lua_key = luaL_checkstring(L, -2);
      count++;
      lua_pop(L, 1);
    }

  /* Use C library qsort */
  
  qsort(klist, count, sizeof(keypair_t), keypair_compare);

  /* Create new table and treat as an array */
  
  lua_newtable(L);
  for (unsigned int i = 0; i < count; i++)
    {
      /* Use string key to get value and then push onto array */
      lua_getfield(L, 1, klist[i].lua_key);
      lua_rawseti(L, -2, i+1);
    }

  /* Free temporary array */
  delete klist;

  /* Return table at the top of the stack */
  
  return 1;
}

/* ----------------------------------------------------------------------
   --
   -- arraystats
   --
   ---------------------------------------------------------------------- */

int hacks_arraystats(lua_State *L)
{

  /* return false if called without parameter or first parameter is not a table */
  
  if ((lua_gettop(L) < 1) || !lua_istable(L, 1))
    {
      lua_pushboolean(L, 0);
      return 1;
    }
  
  int start = -1;
  int array = 1;
  int first = 1;
  int end = -1;
  int size = 0;
  int sparse = 0;
  int index;
  
  lua_pushnil(L);  /* first key */
  while (lua_next(L, 1) != 0)
    {
      /* uses 'key' (at index -2) and 'value' (at index -1) */
      //       printf("%s: %s\n", luaL_typename(L, -2), luaL_typename(L, -1));

      /* If key not an integer then break out */
      
      if (!lua_isinteger(L, -2))
	{
	  array = 0;
	  break;
	}
      index = lua_tointeger(L, -2);
      //      printf("-- %d\n", newindex);
      size++;
      if (first)
	{
	  start = index;
	  end = index;
	  first = 0;
	}
      else
	{
	  if (index != end + 1)
	    sparse = 1;
	  end = index;
	}
      lua_pop(L, 1);
    }
  end = index;
  if (!(array && !first)) /* ! (array and has atleast one element) */
    {
      lua_pushboolean(L, 0);
      return 1;
    }
  
  printf("Array start %d end %d size %d sparse %d\n", start, end, size, sparse);
  return 0;
}

jsoncons::json* hacks_json_indexed(lua_State *L, int index)
{
  int ltype = lua_type(L, index);
  jsoncons::json *res = nullptr;
  switch(ltype)
    {
    case LUA_TNUMBER:
      res = new jsoncons::json(lua_tonumber(L, index));
      break;
    case LUA_TSTRING:
      res = new jsoncons::json(lua_tostring(L, index));
      break;
    case LUA_TTABLE:
      int isarray = 1;
      int count = 0;
      lua_pushnil(L);
      while (lua_next(L, index) != 0)
	{
	  count++;
	  if (!lua_isinteger(L, -2))
	    {
	      isarray = 0;
	      lua_pop(L, 2);
	      break;
	    }
	  lua_pop(L, 1);
	}
      if (isarray && count)
	res = new jsoncons::json(jsoncons::json_array_arg);
      else
	res = new jsoncons::json;
      
      lua_pushnil(L);
      while (lua_next(L, index) != 0)
	{
	  jsoncons::json *value = hacks_json_indexed(L, lua_gettop(L));
	  if (isarray)
	    (*res).push_back(*value);
	  else
	    (*res)[lua_tostring(L, -2)] = *value;
	  delete value;
	  lua_pop(L, 1);
	}
      break;
    }
  return res;
}

int hacks_json(lua_State *L)
{
  std::vector<std::string> res(lua_gettop(L));
  for (unsigned int i = 1; i <= lua_gettop(L); i++)
    {
      jsoncons::json *j = hacks_json_indexed(L, i);
      if (j != nullptr)
	{
	  auto options = jsoncons::json_options{}
	    .escape_all_non_ascii(false);
	  j->dump(res[i-1], options);
	  delete j;
	}
    }
  for (unsigned int i = 0; i < res.size(); i++)
    lua_pushlstring(L, res[i].c_str(), res[i].size());
  return res.size();
}

extern "C" int luaopen_luahacks(lua_State *L)
{
  const luaL_Reg iphacks_methods[] = {
    {"ipkeysort", iphacks_ipkeysort},
    {"arraystats", hacks_arraystats},
    {"json", hacks_json},
    {0, 0}
  };
  /* Notes on parameter, currently not used
     
  const char *modname = luaL_checkstring(L, 1);
  const char *modpath = luaL_checkstring(L, 2);
  */
  
  luaL_newlib(L, iphacks_methods);
  return 1;
}
