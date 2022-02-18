/* ----------------------------------------------------------------------
--
-- tile
--
-- 2019-03-21: Ross Alexander
--   Move depth to tile_surface_t and set it in the constructors
--
---------------------------------------------------------------------- */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_XCB_XCB_H
#include <xcb/xcb.h>
#endif

#ifdef HAVE_XCB_XCBAUX_H
#include <xcb/xcb_aux.h>
#endif

#ifdef HAVE_CAIRO_H
#include <cairo.h>
#endif

#ifdef HAVE_CAIROXLIB_H
#include <cairo-xlib.h>
#endif

#ifdef HAVE_CAIROXCB_H
#include <cairo-xcb.h>
#endif

#ifdef HAVE_GDK30_H
#include <gdk/gdk.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <vector>
#include <string>
#include <algorithm>
#include <lua.hpp>
#include <fmt/format.h>
#include <fmt/printf.h>

#include "tile.h"


/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  const char *luafile = nullptr;

  if (argc < 2)
    {
      fprintf(stderr, "%s: <lua file>\n", argv[0]);
      exit(1);
    }
  luafile = argv[1];

  /* --------------------
     -- init lua
     -------------------- */
  
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  luaopen_tile_surface_t(L);
  luaopen_tile_format_t(L);
  luaopen_tile_pixbuf_t(L);
  
  /* --------------------
     -- Add args to lua
     -------------------- */

  lua_pushglobaltable(L);
  lua_newtable(L);
  
  for (int i = optind; i < argc; i++)
    {
      lua_pushstring(L, argv[i]);
      lua_rawseti(L, -2, i - optind + 1);
    }
  lua_setfield(L, -2, "args");

  lua_pushglobaltable(L);
  luaopen_scanner_t(L);
  lua_setfield(L, -2, "scanner_t");
  
  if (luafile)
    {
      int ret = luaL_dofile(L, luafile);
      if (ret != 0)
	{
	  fprintf(stderr, "%s\n", lua_tostring(L, -1));
	  exit(1);
	}
    }
  lua_pushglobaltable(L);
  lua_getfield(L, -2, "background");
  if (lua_isfunction(L, -1))
    {
      lua_call(L, 0, 0);
    }
  else
    {
      lua_pop(L, 1);
    }
  lua_close(L);
}
