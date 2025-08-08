#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include <arpa/inet.h>

#include <vector>

#include <lua.hpp>

#include "qsort.h"
#include "trie.h"

struct ip_table_entry_t {
  uint32_t addr;
  uint32_t len;
  void *value;
};

typedef std::vector<ip_table_entry_t*> ip_table_t;


/* ----------------------------------------------------------------------
   --
   -- trie_read_file
   --
   ---------------------------------------------------------------------- */

ip_table_t* trie_read_file(const char *path)
{  
  FILE *stream = fopen(path, "r");

  if (stream == NULL)
    {
      fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
      exit(1);
    }

  ip_table_t* table =  new(ip_table_t);
  
  /* Use strtok to split fields */
  
  int line_len = 1024;
  char *line = (char*)calloc(line_len, sizeof(char));
  while (fgets(line, line_len, stream))
    {
      const char* delim = "\t\n";
      char *next_token, *token;
      unsigned int count = 0;

      char *line_dup = strdup(line);
      token = strtok_r(line_dup, delim, &next_token);
      while (token)
	{
	  count++;
	  token = strtok_r(NULL, delim, &next_token);
	}
      free(line_dup);
      int nfields = count;
      char* bits[nfields];

      count = 0;
      token = strtok_r(line, delim, &next_token);
      while (token)
	{
	  bits[count++] = token;
	  token = strtok_r(NULL, delim, &next_token);
	}

      char *addr_field = bits[0];
      char *comment_field;

      if (count > 1)
	comment_field = strdup(bits[1]);
      else
	comment_field = 0;
      
      printf("%s -- %s\n", addr_field, comment_field);

      /* Split prefix into address and length */
      
      delim = "/";
      char *addr[2] = {NULL, NULL};
      count = 0;
      char *addr_field_dup = strdup(addr_field);
      
      token = strtok_r(addr_field_dup, delim, &next_token);
      while (token)
	{
	  count++;
	  token = strtok_r(NULL, delim, &next_token);
	}
      free(addr_field_dup);

      if ((count < 1) || (count > 2))
	{
	  fprintf(stderr, "Entry %s isn't an IP prefix\n", addr_field);
	  exit(1);
	}
      nfields = count;
      count = 0;
      token = strtok_r(addr_field, delim, &next_token);
      while (token)
	{
	  addr[count++] = token;
	  token = strtok_r(NULL, delim, &next_token);
	}

      ip_table_entry_t *entry = new(ip_table_entry_t);

      /* Use ntohl as inet_addr returns address in Network (big) endian */
      
      entry->addr = ntohl(inet_addr(addr[0]));
      if (addr[1])
	entry->len = strtoul(addr[1], NULL, 10);
      else
	entry->len = 32;
      entry->value = comment_field;
      table->push_back(entry);
    }
  fclose(stream);
  return table;
}

/* ----------------------------------------------------------------------
--
-- ip_table
--
---------------------------------------------------------------------- */

int ip_table_entry_t___tostring(lua_State *L)
{
  ip_table_entry_t *t = *(ip_table_entry_t**)luaL_checkudata(L, 1, "ip_table_entry_t");
  int nbuf = 30;
  if (t->value)
    nbuf += strlen((char*)(t->value)) + 2;
  char buf[nbuf];
  struct in_addr a;
  a.s_addr = ntohl(t->addr);
  if (t->value)    
    snprintf(buf, nbuf, "%s/%d\t%s", inet_ntoa(a), t->len, t->value);
  else
    snprintf(buf, nbuf, "%s/%d", inet_ntoa(a), t->len);
  lua_pushstring(L, buf);
  return 1;
}


/* ----------------------------------------------------------------------
--
-- ip_table
--
---------------------------------------------------------------------- */

int ip_table_t___tostring(lua_State *L)
{
  ip_table_t *t = *(ip_table_t**)luaL_checkudata(L, 1, "ip_table_t");
  lua_pushstring(L, "<ip_table_t>");
  return 1;
}

int ip_table_t___len(lua_State *L)
{
  ip_table_t *t = *(ip_table_t**)luaL_checkudata(L, 1, "ip_table_t");
  lua_pushinteger(L, t->size());
  return 1;
}

int ip_table_t___next(lua_State *L)
{
  ip_table_t *t = *(ip_table_t**)luaL_checkudata(L, 1, "ip_table_t");
  
  if ((lua_gettop(L) > 1) && (lua_isnil(L, 2)))
    {
      lua_pushinteger(L, 1);
      ip_table_entry_t** i_p = (ip_table_entry_t**)lua_newuserdata(L, sizeof(ip_table_entry_t*));
      *i_p = (*t)[0];
      lua_getfield(L, LUA_REGISTRYINDEX, "ip_table_entry_t");
      lua_setmetatable(L, -2);
      return(2);
    }
  else
    {
      int index = lua_tointeger(L, 2);
      index++;
      if (index <=t->size())
	{
	  lua_pushinteger(L, index);
	  ip_table_entry_t** i_p = (ip_table_entry_t**)lua_newuserdata(L, sizeof(ip_table_entry_t*));
	  *i_p = (*t)[index - 1];
	  lua_getfield(L, LUA_REGISTRYINDEX, "ip_table_entry_t");
	  lua_setmetatable(L, -2);
	  return(2);
	}
      else
	{
	  lua_pushnil(L);
	  return(1);
	}
    }
}

int ip_table_t___pairs(lua_State *L)
{
  lua_pushcclosure(L, ip_table_t___next, 0);
  lua_pushvalue(L, 1);
  lua_pushnil(L);
  return 3;
}




int trie_lua_read_file(lua_State *L)
{
  if (lua_gettop(L) < 1)
    luaL_error(L, "Called with no parameters");
  const char *path;
  size_t path_len;
  if ((path = luaL_checklstring(L, 1, &path_len)) == NULL)
    luaL_error(L, "read_file expecting a filestring (string)");
  printf("Got path %s\n", path);
  ip_table_t* table = trie_read_file(path);
  printf("Found %d entries\n", table->size());

  ip_table_t **table_p = (ip_table_t**)lua_newuserdata(L, sizeof(ip_table_t*));
  *table_p = table;

  printf("Found %d entries.\n", (*table_p)->size());
  
  lua_getfield(L, LUA_REGISTRYINDEX, "ip_table_t");
  lua_setmetatable(L, -2);

  return 1;
}

int main(int argc, char *argv[])
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  /* --------------------
     ip_table_entry
     -------------------- */

  luaL_newmetatable(L, "ip_table_entry_t");
  lua_pushcclosure(L, ip_table_entry_t___tostring, 0); lua_setfield(L, -2, "__tostring");

  /* --------------------
     ip_table
     -------------------- */

  luaL_newmetatable(L, "ip_table_t");
  lua_pushcclosure(L, ip_table_t___tostring, 0); lua_setfield(L, -2, "__tostring");
  lua_pushcclosure(L, ip_table_t___pairs, 0); lua_setfield(L, -2, "__pairs");
  lua_pushcclosure(L, ip_table_t___len, 0); lua_setfield(L, -2, "__len");

  /* --------------------
     global functions
     -------------------- */

  lua_pushcclosure(L, trie_lua_read_file, 0);
  lua_setglobal(L, "read_file");

  lua_getglobal(L, "read_file");

  /*
  if (argc > 1)
    lua_pushstring(L, argv[1]);
  else
    lua_pushstring(L, "smoothwall-20250807-1601.txt");
  //  lua_pushstring(L, "sites.txt");
  if (lua_pcall(L, 1, 0, 0) != 0)
    {
      fprintf(stderr, "error running function: %s\n", lua_tostring(L, -1));
    }
  */
  if (argc > 1)
    {
      printf("Running lua script %s\n", argv[1]);
      int ret = luaL_dofile(L, argv[1]);
    }  
  return 0;   
}
