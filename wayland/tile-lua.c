#include <assert.h>
#include <stdio.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <cairo.h>
#include <glycin.h>
#include <babl/babl.h>

#include "tile.h"

/* ----------------------------------------------------------------------
   --
   -- meta functions
   --
   ---------------------------------------------------------------------- */

int tile_surface_t___tostring(lua_State *L)
{
  tile_surface_t *tile = *(tile_surface_t**)luaL_checkudata(L, 1, "tile_surface_t");
  int size = snprintf(nullptr, 0, "tile_surface_t(%d × %d)", tile->width, tile->height);
  size += 1;
  char *s = malloc(size);
  snprintf(s, size, "tile_surface_t(%d × %d)", tile->width, tile->height);
  lua_pushstring(L, s);
  free(s);
  return 1;
}

int tile_context_t___tostring(lua_State *L)
{
  lua_pushstring(L, "tile_context_t");
  return 1;
}

/* ----------------------------------------------------------------------
   --
   -- context_t
   --
   ---------------------------------------------------------------------- */

int tile_context_t_rectangle(lua_State *L)
{
  assert(lua_gettop(L) == 5);
  tile_context_t *context = *(tile_context_t**)luaL_checkudata(L, 1, "tile_context_t");
  cairo_rectangle(context->cr,
		  luaL_checknumber(L, 2),
		  luaL_checknumber(L, 3),
		  luaL_checknumber(L, 4),
		  luaL_checknumber(L, 5));
  return 0;
}

int tile_context_t_set_source_rgb(lua_State *L)
{
  assert(lua_gettop(L) == 4);
  tile_context_t *context = *(tile_context_t**)luaL_checkudata(L, 1, "tile_context_t");
  cairo_set_source_rgb(context->cr,
		       luaL_checknumber(L, 2),
		       luaL_checknumber(L, 3),
		       luaL_checknumber(L, 4));
  return 0;
}

int tile_context_t_set_source_surface(lua_State *L)
{
  assert(lua_gettop(L) == 4);
  tile_context_t *context = *(tile_context_t**)luaL_checkudata(L, 1, "tile_context_t");
  tile_surface_t *surface = *(tile_surface_t**)luaL_checkudata(L, 2, "tile_surface_t");
  cairo_set_source_surface(context->cr,
			   surface->surface,
			   luaL_checknumber(L, 3),
			   luaL_checknumber(L, 4));
  return 0;
}


int tile_context_t_fill(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  tile_context_t *context = *(tile_context_t**)luaL_checkudata(L, 1, "tile_context_t");
  cairo_fill(context->cr);
  return 0;
}

int tile_context_t_fill_preserve(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  tile_context_t *context = *(tile_context_t**)luaL_checkudata(L, 1, "tile_context_t");
  cairo_fill_preserve(context->cr);
  return 0;
}

int tile_context_t_stroke(lua_State *L)
{
  assert(lua_gettop(L) == 1);
  tile_context_t *context = *(tile_context_t**)luaL_checkudata(L, 1, "tile_context_t");
  cairo_stroke(context->cr);
  return 0;
}

int tile_context_t_set_line_width(lua_State *L)
{
  assert(lua_gettop(L) == 2);
  tile_context_t *context = *(tile_context_t**)luaL_checkudata(L, 1, "tile_context_t");
  double width = luaL_checknumber(L, 2);
  cairo_set_line_width(context->cr, width);
  return 0;
}

/* ----------------------------------------------------------------------
   --
   -- tile_surface_t_new_from_file
   --
   -- Use the glycin-2 library to do file loading.  This will have
   -- better support than GDK-Pixbuf going forward and is less
   -- problematic then GEGL.
   --
   -- It uses BABL to do any format conversions to CAIRO_ARGB32
   --
   ---------------------------------------------------------------------- */

int tile_surface_t_new_from_file(lua_State *L)
{
  const char *path = luaL_checkstring(L, 1);

  printf("tile_surface_t_new_from_file(%s)\n", path);

  /* --------------------
     Create userdata object
     -------------------- */
  
  tile_surface_t **handle = (tile_surface_t**)lua_newuserdata(L, sizeof(tile_surface_t*));
  tile_surface_t *tile = *handle = tile_surface_new_from_file(path); // calloc(sizeof(tile_surface_t), 1);
  luaL_setmetatable(L, "tile_surface_t");
  return 1;
}

/* ----------------------------------------------------------------------
   --
   -- tile_surface_t_new
   --
   -- Create blank surface from supplied dimensions
   --
   ---------------------------------------------------------------------- */

int tile_surface_t_new(lua_State *L)
{
  /* Check width & height */
  assert(lua_gettop(L) == 2);
  int width = luaL_checkinteger(L, 1);
  int height = luaL_checkinteger(L, 2);

  tile_surface_t **handle = (tile_surface_t**)lua_newuserdata(L, sizeof(tile_surface_t*));
  *handle = tile_surface_new(width, height);
  luaL_setmetatable(L, "tile_surface_t");

  /* create blank and transparent surface */
  
  //  tile->width = width;
  //  tile->height = height;

  return 1;
}

/* ----------------------------------------------------------------------
   --
   -- tile_surface_t_to_png(lua_State *L)
   --
   ---------------------------------------------------------------------- */

int tile_surface_t_write_to_png(lua_State *L)
{
  tile_surface_t *tile = *(tile_surface_t**)luaL_checkudata(L, 1, "tile_surface_t");
  const char *path = luaL_checkstring(L, 2);

  assert(tile->width);
  assert(tile->height);
  assert(tile->surface);

  cairo_status_t status = cairo_surface_write_to_png(tile->surface, path);
  if (status == CAIRO_STATUS_INVALID_CONTENT)
    {
      fprintf(stderr, "Surface has invalid content.\n");
    }
  if (status == CAIRO_STATUS_INVALID_FORMAT)
    {
      fprintf(stderr, "Surface has invalid format.\n");
    }
  lua_pushboolean(L, status == CAIRO_STATUS_SUCCESS ? 1 : 0);
  return 1;
}

int tile_surface_t_configuration(lua_State *L)
{
  tile_surface_t* tile = *(tile_surface_t**)luaL_checkudata(L, 1, "tile_surface_t");
  lua_newtable(L);
  lua_pushinteger(L, tile->width);
  lua_setfield(L, -2, "width");
  lua_pushinteger(L, tile->height);
  lua_setfield(L, -2, "height");
  return 1;
}

int tile_surface_t_context_create(lua_State *L)
{
  tile_surface_t *tile = *(tile_surface_t**)luaL_checkudata(L, 1, "tile_surface_t");

  tile_context_t **handle = (tile_context_t**)lua_newuserdata(L, sizeof(tile_context_t*));
  tile_context_t *context = *handle = calloc(sizeof(tile_surface_t), 1);
  luaL_setmetatable(L, "tile_context_t");
  
  context->cr = cairo_create(tile->surface);
  return 1;
}


/* ----------------------------------------------------------------------
   --
   -- luaopen_tile
   --
   -- Called by lua when loaded as a shared library from require
   --
   ---------------------------------------------------------------------- */

int luaopen_tile(lua_State *L)
{
  /* If called from require and pulled in as a shared library then
     the stack has the module name and the path to the library.
     If called from luaL_requiref then only the module name is
     passed on the stack */

  babl_init();

  printf("openlib_tile with %d objects on the stack.\n", lua_gettop(L));
  for (unsigned int i = 1; i <= lua_gettop(L); i++)
    {
      const char *s;
      if ((s = luaL_checkstring(L, i)) != nullptr)
	printf("%d - %s\n", i, s);
    }

  /* --------------------
     tile_context_t
     -------------------- */

  const luaL_Reg tile_context_t_meta_methods[] = {
    {"__tostring", tile_context_t___tostring},
    {0, 0},
  };

  luaL_newmetatable(L, "tile_context_t");
  luaL_setfuncs(L, tile_context_t_meta_methods, 0);

  const luaL_Reg tile_context_t_instance_methods[] = {
    {"rectangle",		tile_context_t_rectangle},
    {"set_source_rgb",		tile_context_t_set_source_rgb},
    {"set_source_surface",	tile_context_t_set_source_surface},
    {"fill",			tile_context_t_fill},
    {"fill_preserve",	       	tile_context_t_fill_preserve},
    {"stroke",			tile_context_t_stroke},
    {"set_line_width",		tile_context_t_set_line_width},
    {0, 0}
  };
  
  lua_newtable(L);
  luaL_setfuncs(L, tile_context_t_instance_methods, 0);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  /* --------------------
     tile_surface_t
     -------------------- */
  
  const luaL_Reg tile_surface_t_meta_methods[] = {
    {"__tostring", tile_surface_t___tostring},
    {0, 0},
  };

  luaL_newmetatable(L, "tile_surface_t");
  luaL_setfuncs(L, tile_surface_t_meta_methods, 0);

  const luaL_Reg tile_surface_t_instance_methods[] = {
    {"write_to_png",	       	tile_surface_t_write_to_png},
    {"context_create",		tile_surface_t_context_create},
    {"configuration",		tile_surface_t_configuration},
    {0, 0}
  };

  lua_newtable(L);
  luaL_setfuncs(L, tile_surface_t_instance_methods, 0);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1); // metatable

  const luaL_Reg tile_surface_t_class_methods[] = {
    {"new",			tile_surface_t_new},
    {"new_from_file",		tile_surface_t_new_from_file},
    { 0, 0}
  };
  lua_newtable(L);
  luaL_setfuncs(L, tile_surface_t_class_methods, 0); 
  return 1;
}
