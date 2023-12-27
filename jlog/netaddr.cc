#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <lua.hpp>

/* ----------------------------------------------------------------------
--
-- NetAddr
--
---------------------------------------------------------------------- */

struct v4mask {
  const char *addr, *mask;
} rfc1918[] = {
  {"10.0.0.0", "255.0.0.0"},
  {"172.16.0.0", "255.240.0.0"},
  {"192.168.0.0", "255.255.0.0"},
  {0, 0},
};

int NetAddr_new(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isstring(L, 1));

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo** ai = (struct addrinfo**)lua_newuserdata(L, sizeof(struct addrinfo*));

  int res;
  if ((res = getaddrinfo(lua_tostring(L, 1), 0, &hints, ai)) != 0)
    {
      fprintf(stderr, "gai_error(%s)\n", gai_strerror(res));
      exit(1);
    }
  lua_getfield(L, LUA_REGISTRYINDEX, "NetAddr");
  lua_setmetatable(L, -2);
  return 1;
}

int NetAddr___tostring(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isuserdata(L, 1));

  struct addrinfo* ai = *((struct addrinfo**)lua_touserdata(L, 1));

  char host[1024];
  getnameinfo(ai->ai_addr, ai->ai_addrlen, host, sizeof(host), 0, 0, NI_NUMERICHOST);

  lua_pushstring(L, host);
  return 1;
}

int NetAddr_resolve(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isuserdata(L, 1));

  struct addrinfo* ai = *((struct addrinfo**)lua_touserdata(L, 1));

  char host[1024];
  getnameinfo(ai->ai_addr, ai->ai_addrlen, host, sizeof(host), 0, 0, 0);

  lua_pushstring(L, host);
  return 1;
}



int NetAddr_rfc1918(lua_State *L)
{
  assert(lua_gettop(L) > 0);
  assert(lua_isuserdata(L, 1));
  struct addrinfo* ai = *((struct addrinfo**)lua_touserdata(L, 1));

  int res = 0;

  if (ai->ai_family == AF_INET)
    {
      for (int i = 0; rfc1918[i].addr; i++)
	{
	  uint32_t rfc10, addr, mask;
	  inet_pton(AF_INET, rfc1918[i].addr, &rfc10);
	  inet_pton(AF_INET, rfc1918[i].mask, &mask);
	  sockaddr_in *in = (sockaddr_in*)ai->ai_addr;
	  addr = ntohl(in->sin_addr.s_addr) & ntohl(mask);
	  rfc10 = ntohl(rfc10);
	  mask = ntohl(mask);
	  res |= (rfc10 == addr);
	}
      //      printf("%08x %08x %08x\n", rfc10, addr, mask);
    }
  lua_pushboolean(L, res);
  return 1;
}

int NetAddr___gc(lua_State *L)
{
  struct addrinfo* ai = *((struct addrinfo**)lua_touserdata(L, 1));
  freeaddrinfo(ai);
  return 0;
}

static int NetAddr_cmp(lua_State *L)
{
  assert(lua_gettop(L) >= 2);
  struct addrinfo* a = *((struct addrinfo**)lua_touserdata(L, -2));
  struct addrinfo* b = *((struct addrinfo**)lua_touserdata(L, -1));

  if (a->ai_family < b->ai_family)
    return -1;
  else if (a->ai_family > b->ai_family)
    return 1;

  return memcmp(a->ai_addr, b->ai_addr, a->ai_addrlen);
}

int NetAddr___lt(lua_State *L)
{
  int ret = NetAddr_cmp(L);
  return NetAddr_cmp(L) < 0 ? 1 : 0;
}

int NetAddr___eq(lua_State *L)
{
  int ret = NetAddr_cmp(L);
  return NetAddr_cmp(L) == 0 ? 1 : 0;
}


extern "C" int luaopen_NetAddr(lua_State *L)
{
  /* --------------------
     Create NetAddr metatable
     -------------------- */

  lua_newtable(L);
  lua_pushcclosure(L, NetAddr___tostring, 0); lua_setfield(L, -2, "__tostring");
  lua_pushcclosure(L, NetAddr___gc, 0); lua_setfield(L, -2, "__gc");
  lua_pushcclosure(L, NetAddr___lt, 0); lua_setfield(L, -2, "__lt");
  lua_pushcclosure(L, NetAddr___eq, 0); lua_setfield(L, -2, "__eq");

  lua_newtable(L);
  lua_pushcclosure(L, NetAddr_rfc1918, 0); lua_setfield(L, -2, "rfc1918");
  lua_pushcclosure(L, NetAddr_resolve, 0); lua_setfield(L, -2, "resolve");

  lua_setfield(L, -2, "__index");

  lua_setfield(L, LUA_REGISTRYINDEX, "NetAddr");

  /* --------------------
     Create NetAddr object
     -------------------- */

  lua_newtable(L);
  lua_pushcclosure(L, NetAddr_new, 0);
  lua_setfield(L, -2, "new");
  return 1;
}

