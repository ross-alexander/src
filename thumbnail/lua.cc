#include <string>
#include <map>
#include <iostream>
#include <filesystem>

#include <assert.h>

#include <fmt/printf.h>

#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo.h>
#include <lua.hpp>
#include <boost/program_options.hpp>

#include "image.h"
#include "thumbnail.h"

typedef std::map<std::string, image_t*> image_table_t;

/* ----------------------------------------------------------------------
--
-- image_t (pixbuf)
--
---------------------------------------------------------------------- */

int pixbuf_t_new_from_file(lua_State *L)
{
  const char *path = lua_tostring(L, 1);
  image_t** ip = (image_t**)lua_newuserdata(L, sizeof(image_t*));
  *ip = new pixbuf_t(path);
  lua_getfield(L, LUA_REGISTRYINDEX, "image_t");
  lua_setmetatable(L, -2);
  return 1;
}

int pixbuf_t_new(lua_State *L)
{
  image_t** ip = (image_t**)lua_newuserdata(L, sizeof(image_t*));
  *ip = new pixbuf_t(lua_tointeger(L, 1), lua_tointeger(L, 2));

  lua_getfield(L, LUA_REGISTRYINDEX, "image_t");
  lua_setmetatable(L, -2);
  return 1;
}


/* ----------------------------------------------------------------------
--
-- image_t
--
---------------------------------------------------------------------- */

int image_t_new_from_file(lua_State *L)
{
  const char *path = lua_tostring(L, 1);
  image_t** ip = (image_t**)lua_newuserdata(L, sizeof(image_t*));
  *ip = new gegl_t(path);
  lua_getfield(L, LUA_REGISTRYINDEX, "image_t");
  lua_setmetatable(L, -2);
  return 1;
}

int image_t_new(lua_State *L)
{
  image_t** ip = (image_t**)lua_newuserdata(L, sizeof(image_t*));
  *ip = new gegl_t(lua_tointeger(L, 1), lua_tointeger(L, 2));
  lua_getfield(L, LUA_REGISTRYINDEX, "image_t");
  lua_setmetatable(L, -2);
  return 1;
}

int image_t___tostring(lua_State *L)
{
  image_t *image = *(image_t**)lua_touserdata(L, 1);
  std::string s = image->description();
  lua_pushstring(L, s.c_str());
  return 1;
}

int image_t___index(lua_State *L)
{
  assert(lua_gettop(L) == 2);
  assert(luaL_checkudata(L, 1, "image_t"));
  const char* key = luaL_checkstring(L, 2);

  lua_pushvalue(L, lua_upvalueindex(1));
  lua_getfield(L, -1, key);
  if(!lua_isnil(L, -1))
    return 1;

  lua_pushvalue(L, lua_upvalueindex(2));
  lua_getfield(L, -1, key);
  
  if (!lua_isnil(L, -1))
    {
      lua_pushvalue(L, 1);
      lua_call(L, 1, 1);
    }
  return 1;
}

int image_t_save(lua_State *L)
{
  image_t *image = *(image_t**)luaL_checkudata(L, 1, "image_t");
  if (lua_isstring(L, 2))
    image->save(lua_tostring(L, 2));
  else
    image->save();
  return 0;
}

int image_t_fill(lua_State *L)
{
  image_t *image = *(image_t**)lua_touserdata(L, 1);
  image->fill(lua_tostring(L, 2));
  return 0;
}

int image_t_frame(lua_State *L)
{
  image_t *image = *(image_t**)luaL_checkudata(L, 1, "image_t");

  double x = lua_tonumber(L, 2);
  double y = lua_tonumber(L, 3);
  double w = lua_tonumber(L, 4);
  double h = lua_tonumber(L, 5);
  double l = lua_tonumber(L, 6);

  image->frame(x, y, w, h, l, lua_tostring(L, 7));
  return 0;
}

int image_t_scale_to(lua_State *L)
{
  image_t *src = *(image_t**)luaL_checkudata(L, 1, "image_t");
  image_t *dst = src->scale((int)(lua_tointeger(L, 2)));
  image_t** ip = (image_t**)lua_newuserdata(L, sizeof(image_t*));
  *ip = dst;
  lua_getfield(L, LUA_REGISTRYINDEX, "image_t");
  lua_setmetatable(L, -2);
  return 1;
}

int image_t_scale(lua_State *L)
{
  image_t *src = *(image_t**)luaL_checkudata(L, 1, "image_t");
  double scale = lua_tonumber(L, 2);
  image_t *dst = src->scale((double)scale);
  image_t** ip = (image_t**)lua_newuserdata(L, sizeof(image_t*));
  *ip = dst;
  lua_getfield(L, LUA_REGISTRYINDEX, "image_t");
  lua_setmetatable(L, -2);
  return 1;
}


int image_t_compose(lua_State *L)
{
  image_t *dst = *(image_t**)lua_touserdata(L, 1);
  image_t *src = *(image_t**)lua_touserdata(L, 2);

  dst->compose(src, lua_tonumber(L, 3), lua_tonumber(L, 4));
  return 1;
}

int image_t_width_get(lua_State *L)
{
  image_t* i = *(image_t**)luaL_checkudata(L, 1, "image_t");
  lua_pushinteger(L, i->width());
  return 1;
}

int image_t_height_get(lua_State *L)
{
  image_t* i = *(image_t**)luaL_checkudata(L, 1, "image_t");
  lua_pushinteger(L, i->height());
  return 1;
}

int image_t_valid_get(lua_State *L)
{
  image_t* i = *(image_t**)luaL_checkudata(L, 1, "image_t");
  lua_pushboolean(L, i->is_valid());
  return 1;
}

int image_t_add_text(lua_State *L)
{
  image_t* i = *(image_t**)luaL_checkudata(L, 1, "image_t");
  double x = lua_tonumber(L, 2);
  double y = lua_tonumber(L, 3);
  double width = lua_tonumber(L, 4);
  double fontsize = lua_tonumber(L, 5);
  const char *family = lua_tostring(L, 6);
  const char *string = lua_tostring(L, 7);

  printf("add_text(%f %f %f %f %s %s\n", x, y, width, fontsize, family, string);
  
  bounds_t bb = i->text(x, y, width, fontsize, family, string);
  
  lua_newtable(L);
  lua_pushnumber(L, bb.x); lua_setfield(L, -2, "x");
  lua_pushnumber(L, bb.y); lua_setfield(L, -2, "y");
  lua_pushnumber(L, bb.width); lua_setfield(L, -2, "width");
  lua_pushnumber(L, bb.height); lua_setfield(L, -2, "height");

  return 1;
}
  
int image_t_name(lua_State *L)
{
  image_t* i = *(image_t**)luaL_checkudata(L, 1, "image_t");
  if (i->path.empty())
    lua_pushnil(L);
  else
    lua_pushstring(L, i->path.filename().c_str());
  return 1;
}

/* ----------------------------------------------------------------------
--
-- image_table
--
---------------------------------------------------------------------- */

int image_table_t___tostring(lua_State *L)
{
  image_table_t *t = *(image_table_t**)luaL_checkudata(L, 1, "image_table_t");
  for(auto &i : *t)
    {
      printf("+- %s\n", i.first.c_str());
    }
  lua_pushstring(L, "<image_table_t>");
  return 1;
}

int image_table_t_remove(lua_State *L)
{
  image_table_t *t = *(image_table_t**)luaL_checkudata(L, 1, "image_table_t");
  assert(lua_gettop(L) == 2);
  const char *key = luaL_checkstring(L, 2);
  t->erase(key);
  return 0;
}

int image_table_t___len(lua_State *L)
{
  image_table_t *t = *(image_table_t**)luaL_checkudata(L, 1, "image_table_t");
  lua_pushinteger(L, t->size());
  return 1;
}

int image_table_t___next(lua_State *L)
{
  image_table_t *t = *(image_table_t**)luaL_checkudata(L, 1, "image_table_t");
  
  if ((lua_gettop(L) > 1) && (lua_isnil(L, 2)))
    {
      auto i = t->begin();
      lua_pushstring(L, i->first.c_str());
      if (i->second != nullptr)
	{
	  image_t** i_p = (image_t**)lua_newuserdata(L, sizeof(image_t*));
	  *i_p = i->second;
	  lua_getfield(L, LUA_REGISTRYINDEX, "image_t");
	  lua_setmetatable(L, -2);
	}
      else
	{
	  lua_pushnil(L);
	}
      return(2);
    }
  else
    {
      auto i = t->find(lua_tostring(L, 2));
      i++;
      if (i != t->end())
	{
	  lua_pushstring(L, i->first.c_str());
	  if (i->second != nullptr)
	    {
	      image_t** i_p = (image_t**)lua_newuserdata(L, sizeof(image_t*));
	      *i_p = i->second;
	      lua_getfield(L, LUA_REGISTRYINDEX, "image_t");
	      lua_setmetatable(L, -2);
	    }
	  else
	    {
	      lua_pushnil(L);
	    }
	  return(2);
	}
      else
	{
	  lua_pushnil(L);
	  return(1);
	}
    }
}

int image_table_t___pairs(lua_State *L)
{
  lua_pushcclosure(L, image_table_t___next, 0);
  lua_pushvalue(L, 1);
  lua_pushnil(L);
  return 3;
}

/* ----------------------------------------------------------------------
--
-- thumbnail_t
--
---------------------------------------------------------------------- */

int thumbnail_t___index(lua_State *L)
{
  assert(lua_gettop(L) == 2);
  assert(luaL_checkudata(L, 1, "thumbnail_t"));
  const char* key = luaL_checkstring(L, 2);

  lua_pushvalue(L, lua_upvalueindex(1));
  lua_getfield(L, -1, key);
  if(!lua_isnil(L, -1))
    return 1;

  lua_pushvalue(L, lua_upvalueindex(2));
  lua_getfield(L, -1, key);
  
  if (!lua_isnil(L, -1))
    {
      lua_pushvalue(L, 1);
      lua_call(L, 1, 1);
    }
  return 1;
}

int thumbnail_t_new(lua_State *L)
{
  thumbnail_t** tnp = (thumbnail_t**)lua_newuserdata(L, sizeof(thumbnail_t*));
  *tnp = new thumbnail_t;
  lua_getfield(L, LUA_REGISTRYINDEX, "thumbnail_t");
  lua_setmetatable(L, -2);
  return 1;
}

int thumbnail_t_dir_add(lua_State *L)
{
  luaL_checkudata(L, 1, "thumbnail_t");
  thumbnail_t* tn = *(thumbnail_t**)lua_touserdata(L, 1);

  const char *path = lua_tostring(L, 2);
  tn->dir_add(path);
  return 0;
}

int thumbnail_t_dir_scan(lua_State *L)
{
  luaL_checkudata(L, 1, "thumbnail_t");
  thumbnail_t* tn = *(thumbnail_t**)lua_touserdata(L, 1);
  tn->dir_scan();
  return 0;
}

int thumbnail_t_image_table_get(lua_State *L)
{
  thumbnail_t* tn = *(thumbnail_t**)luaL_checkudata(L, 1, "thumbnail_t");
  image_table_t **it = (image_table_t**)lua_newuserdata(L, sizeof(image_table_t*));
  *it = &tn->image_table;
  lua_getfield(L, LUA_REGISTRYINDEX, "image_table_t");
  lua_setmetatable(L, -2);
  return 1;
}

int thumbnail_t_get_bounds(lua_State *L)
{
  thumbnail_t* tn = *(thumbnail_t**)luaL_checkudata(L, 1, "thumbnail_t");
  int index = 1;
  lua_newtable(L);
  for (auto &i : tn->image_table)
    {
      if (i.second == nullptr)
	{
	  gint width, height;
	  GdkPixbufFormat *format = gdk_pixbuf_get_file_info(i.first.c_str(), &width, &height);
	  if (format)
	    {
	      bounds_t** bounds = (bounds_t**)lua_newuserdata(L, sizeof(bounds_t*));
	      *bounds = new bounds_t();
	      (*bounds)->x = (*bounds)->y = 0;
	      (*bounds)->width = width;
	      (*bounds)->height = height;
	      lua_getfield(L, LUA_REGISTRYINDEX, "bounds_t");
	      lua_setmetatable(L, -2);
	      lua_rawseti(L, -2, index++);
	    }
	}
      else
	{
	  image_t* image = i.second;
	  bounds_t** bounds = (bounds_t**)lua_newuserdata(L, sizeof(bounds_t*));
	  *bounds = new bounds_t();
	  (*bounds)->x = (*bounds)->y = 0;
	  (*bounds)->width = image->width();
	  (*bounds)->height = image->height();
	  lua_getfield(L, LUA_REGISTRYINDEX, "bounds_t");
	  lua_setmetatable(L, -2);
	  lua_rawseti(L, -2, index++);
	}
    }
  return 1;
}

/* ----------------------------------------------------------------------
   --
   -- validate
   --
   ---------------------------------------------------------------------- */

int thumbnail_t_validate(lua_State *L)
{
  thumbnail_t* tn = *(thumbnail_t**)luaL_checkudata(L, 1, "thumbnail_t");
  tn->validate();
  return 0;
}

int thumbnail_t_load_images(lua_State *L)
{
  thumbnail_t* tn = *(thumbnail_t**)luaL_checkudata(L, 1, "thumbnail_t");
  for (auto &i : tn->image_table)
    {
      if (i.second == nullptr)
	{
	  image_t *image = i.second = new gegl_t();
	  image->path = std::string(i.first);
	}
      i.second->load();
    }
  return 0;
}

int thumbnail_t_load_images_pixbuf(lua_State *L)
{
  thumbnail_t* tn = *(thumbnail_t**)luaL_checkudata(L, 1, "thumbnail_t");
  for (auto &i : tn->image_table)
    {
      if (i.second == nullptr)
	{
	  image_t *image = i.second = new pixbuf_t();
	  image->path = std::string(i.first);
	}
      i.second->load();
    }
  return 0;
}

int thumbnail_t_split(lua_State *L)
{
  thumbnail_t* tn = *(thumbnail_t**)luaL_checkudata(L, 1, "thumbnail_t");
  thumbnail_t *current = nullptr;
  int max = luaL_checkinteger(L, 2);
  int count = 0;
  int index = 1;
  lua_newtable(L);
  for (auto &i : tn->image_table)
    {
      if (current == nullptr)
	{
	  thumbnail_t** tnp = (thumbnail_t**)lua_newuserdata(L, sizeof(thumbnail_t*));
	  *tnp = current = new thumbnail_t;
	  lua_getfield(L, LUA_REGISTRYINDEX, "thumbnail_t");
	  lua_setmetatable(L, -2);
	  lua_rawseti(L, -2, index++);
	}
      current->image_table[i.first] = i.second;
      if (++count >= max)
	{
	  count = 0;
	  current = nullptr;
	}
    }
  return 1;
}

/* ----------------------------------------------------------------------
--
-- image_t_get_file_info
--
---------------------------------------------------------------------- */

int image_t_get_file_info(lua_State *L)
{
  const char* path = luaL_checkstring(L, 1);
  gint w, h;
  GdkPixbufFormat *format = gdk_pixbuf_get_file_info(path, &w, &h);
  if (!format)
    {
      lua_pushnil(L);
      return 1;
    }
  lua_newtable(L);
  lua_pushinteger(L, w); lua_setfield(L, -2, "width");
  lua_pushinteger(L, h); lua_setfield(L, -2, "height");
  return 1;
}

/* ----------------------------------------------------------------------
--
-- bounds_t_new
--
---------------------------------------------------------------------- */

int bounds_t_new(lua_State *L)
{
  bounds_t** bounds = (bounds_t**)lua_newuserdata(L, sizeof(bounds_t*));
  *bounds = new bounds_t();
  (*bounds)->x = (*bounds)->y = (*bounds)->width = (*bounds)->height = 0.0;
  lua_getfield(L, LUA_REGISTRYINDEX, "bounds_t");
  lua_setmetatable(L, -2);
  return 1;
}

int bounds_t___tostring(lua_State *L)
{
  bounds_t *bounds = *(bounds_t**)lua_touserdata(L, 1);
  std::string s = fmt::sprintf("[%s] %4.2f×%4.2f + %4.2f×%4.2f", typeid(*bounds).name(), bounds->x, bounds->y,bounds->width, bounds->height);
  lua_pushstring(L, s.c_str());
  return 1;
}

/* Getter function */

int bounds_t___index(lua_State *L)
{
  assert(lua_gettop(L) == 2);
  assert(luaL_checkudata(L, 1, "bounds_t"));
  const char* key = luaL_checkstring(L, 2);

  /* methods table */
  
  lua_pushvalue(L, lua_upvalueindex(1));
  lua_getfield(L, -1, key);
  if(!lua_isnil(L, -1))
    return 1;

  /* getter table */
  
  lua_pushvalue(L, lua_upvalueindex(2));
  lua_getfield(L, -1, key);
  
  if (!lua_isnil(L, -1))
    {
      lua_pushvalue(L, 1);
      lua_call(L, 1, 1);
    }
  return 1;
}

int bounds_t___newindex(lua_State *L)
{
  assert(lua_gettop(L) == 3);
  assert(luaL_checkudata(L, 1, "bounds_t"));
  const char* key = luaL_checkstring(L, 2);

  lua_pushvalue(L, lua_upvalueindex(1));
  lua_getfield(L, -1, key);
  if(!lua_isnil(L, -1))
    {
      lua_pushvalue(L, 1);
      lua_pushvalue(L, 3);
      lua_call(L, 2, 1);
    }
  return 0;
}



int bounds_t_width_get(lua_State *L)
{
  bounds_t* b = *(bounds_t**)luaL_checkudata(L, 1, "bounds_t");
  lua_pushnumber(L, b->width);
  return 1;
}

int bounds_t_height_get(lua_State *L)
{
  bounds_t* b = *(bounds_t**)luaL_checkudata(L, 1, "bounds_t");
  lua_pushnumber(L, b->height);
  return 1;
}

int bounds_t_width_set(lua_State *L)
{
  bounds_t* b = *(bounds_t**)luaL_checkudata(L, 1, "bounds_t");
  double d = luaL_checknumber(L, 2);
  b->width = d;
  return 0;
}

int bounds_t_height_set(lua_State *L)
{
  bounds_t* b = *(bounds_t**)luaL_checkudata(L, 1, "bounds_t");
  double d = luaL_checknumber(L, 2);
  b->height = d;
  return 0;
}

/* ----------------------------------------------------------------------
--
-- thumbnail_lua_options
--
---------------------------------------------------------------------- */

extern void path_t_init(lua_State *L);

void thumbnail_lua_options(boost::program_options::variables_map options)
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  /* --------------------
     thumbnail_t
     -------------------- */

  /* Push new metatable */
  
  luaL_newmetatable(L, "thumbnail_t");

  /* Push methods table */

  lua_newtable(L);
  lua_pushcclosure(L, thumbnail_t_dir_add, 0); lua_setfield(L, -2, "dir_add");
  lua_pushcclosure(L, thumbnail_t_dir_scan, 0); lua_setfield(L, -2, "dir_scan");
  lua_pushcclosure(L, thumbnail_t_split, 0);  lua_setfield(L, -2, "split");
  lua_pushcclosure(L, thumbnail_t_get_bounds, 0); lua_setfield(L, -2, "get_bounds");
  lua_pushcclosure(L, thumbnail_t_load_images, 0); lua_setfield(L, -2, "load_images");
  lua_pushcclosure(L, thumbnail_t_load_images_pixbuf, 0); lua_setfield(L, -2, "load_images_pixbuf");
  lua_pushcclosure(L, thumbnail_t_validate, 0); lua_setfield(L, -2, "validate");

  /* Getters / Setters table */

  lua_newtable(L);
  lua_pushcclosure(L, thumbnail_t_image_table_get, 0); lua_setfield(L, -2, "image_table");

  /* Set __index metamethod to function with two upvalues */
  
  lua_pushcclosure(L, thumbnail_t___index, 2); lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
  
  /* Add thumbnail_t to global */
  
  lua_pushglobaltable(L);
  lua_newtable(L);
  lua_pushcclosure(L, thumbnail_t_new, 0); lua_setfield(L, -2, "new");
  lua_setfield(L, -2, "thumbnail_t");
  lua_pop(L, 1);

  /* --------------------
     bounds_t
     -------------------- */

  luaL_newmetatable(L, "bounds_t");
  lua_pushcclosure(L, bounds_t___tostring, 0); lua_setfield(L, -2, "__tostring");

  /* instance methods - currently empty */
  lua_newtable(L);
  
  /* Getters table */
  lua_newtable(L);
  lua_pushcclosure(L, bounds_t_width_get, 0); lua_setfield(L, -2, "width");
  lua_pushcclosure(L, bounds_t_height_get, 0); lua_setfield(L, -2, "height");

  /* Set __index metamethod to function with two upvalues */

  lua_pushcclosure(L, bounds_t___index, 2); lua_setfield(L, -2, "__index");

  /* Setters table */

  lua_newtable(L);
  lua_pushcclosure(L, bounds_t_width_set, 0); lua_setfield(L, -2, "width");
  lua_pushcclosure(L, bounds_t_height_set, 0); lua_setfield(L, -2, "height");
  lua_pushcclosure(L, bounds_t___newindex, 1); lua_setfield(L, -2, "__newindex");
  
  lua_pop(L, 1);
  
  /* class methods */
  
  lua_pushglobaltable(L);
  lua_newtable(L);
  lua_pushcclosure(L, bounds_t_new, 0); lua_setfield(L, -2, "new");
  lua_setfield(L, -2, "bounds_t");
  lua_pop(L, 1);


  /* --------------------
     image_t
     -------------------- */

  luaL_newmetatable(L, "image_t");
  lua_pushcclosure(L, image_t___tostring, 0); lua_setfield(L, -2, "__tostring");

  /* Push methods table */

  lua_newtable(L);
  lua_pushcclosure(L, image_t_save, 0); lua_setfield(L, -2, "save");
  lua_pushcclosure(L, image_t_scale, 0); lua_setfield(L, -2, "scale");
  lua_pushcclosure(L, image_t_scale_to, 0); lua_setfield(L, -2, "scale_to");
  lua_pushcclosure(L, image_t_fill, 0); lua_setfield(L, -2, "fill");
  lua_pushcclosure(L, image_t_compose, 0); lua_setfield(L, -2, "compose");
  lua_pushcclosure(L, image_t_frame, 0); lua_setfield(L, -2, "frame");
  lua_pushcclosure(L, image_t_add_text, 0); lua_setfield(L, -2, "add_text");
  lua_pushcclosure(L, image_t_name, 0); lua_setfield(L, -2, "name");
  //   lua_pushcclosure(L, image_t_bounding_box, 0); lua_setfield(L, -2, "bounding_box");
  //   lua_pushcclosure(L, image_t_bounding_box, 0); lua_setfield(L, -2, "bounding_box");

  /* Getters table */

  lua_newtable(L);
  lua_pushcclosure(L, image_t_width_get, 0); lua_setfield(L, -2, "width");
  lua_pushcclosure(L, image_t_height_get, 0); lua_setfield(L, -2, "height");
  lua_pushcclosure(L, image_t_valid_get, 0); lua_setfield(L, -2, "valid");
  //   lua_pushcclosure(L, image_t_has_buffer_get, 0); lua_setfield(L, -2, "has_buffer");

  /* Set __index metamethod to function with two upvalues */
  
  lua_pushcclosure(L, image_t___index, 2); lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  // class methods
  
  lua_pushglobaltable(L);
  lua_newtable(L);
  lua_pushcclosure(L, pixbuf_t_new, 0); lua_setfield(L, -2, "new_pixbuf");
  lua_pushcclosure(L, pixbuf_t_new_from_file, 0); lua_setfield(L, -2, "new_pixbuf_from_file");
  lua_pushcclosure(L, image_t_new, 0); lua_setfield(L, -2, "new");
  lua_pushcclosure(L, image_t_new_from_file, 0); lua_setfield(L, -2, "new_from_file");
  lua_pushcclosure(L, image_t_get_file_info, 0); lua_setfield(L, -2, "get_file_info");
  lua_setfield(L, -2, "image_t");
  lua_pop(L, 1);

  
  /* --------------------
     image_table
     -------------------- */

  luaL_newmetatable(L, "image_table_t");
  lua_pushcclosure(L, image_table_t___tostring, 0); lua_setfield(L, -2, "__tostring");
  lua_pushcclosure(L, image_table_t___pairs, 0); lua_setfield(L, -2, "__pairs");
  lua_pushcclosure(L, image_table_t___len, 0); lua_setfield(L, -2, "__len");

  lua_newtable(L);
  lua_pushcclosure(L, image_table_t_remove, 0); lua_setfield(L, -2, "remove");
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  path_t_init(L);
  
  /* --------------------
     options
     -------------------- */

  lua_pushglobaltable(L);
  lua_newtable(L);
  if (options.count("size"))
    {
      lua_pushinteger(L, options["size"].as<int>());
      lua_setfield(L, -2, "size");
    }
  if (options.count("src"))
    {
      lua_pushstring(L, options["src"].as<std::string>().c_str());
      lua_setfield(L, -2, "src");
    }
  if (options.count("dst"))
    {
      lua_pushstring(L, options["dst"].as<std::string>().c_str());
      lua_setfield(L, -2, "dst");
    }
  lua_setfield(L, -2, "options");
  lua_pop(L, 1);

  /* --------------------
     dofile
     -------------------- */
  
  int ret = luaL_dofile(L, options["lua"].as<std::string>().c_str());
  if (ret != 0)
    {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
      exit(1);
    }
}
