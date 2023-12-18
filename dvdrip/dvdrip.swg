%module dvd

%include "std_map.i"
%include "std_vector.i"
%include "std_string.i"
%include "typemaps.i"

%{
#include <iostream>

#include <lua/5.4/lua.hpp>
#include "dvdrip.h"

extern "C" int luaopen_dvd(lua_State*);  
%}

%define LLDB_NUMBER_TYPEMAP(TYPE)

// Primitive integer mapping
%typemap(in,checkfn="lua_isinteger") TYPE
%{ $1 = ($type)lua_tointeger(L, $input); %}
%typemap(in,checkfn="lua_isinteger") const TYPE&($basetype temp)
%{ temp=($basetype)lua_tointeger(L,$input); $1=&temp;%}
%typemap(out) TYPE
%{ lua_pushinteger(L, (lua_Integer) $1); SWIG_arg++;%}
%typemap(out) const TYPE&
%{ lua_pushinteger(L, (lua_Integer) $1); SWIG_arg++;%}


// Pointer and reference mapping
%typemap(in,checkfn="lua_isinteger") TYPE *INPUT($*ltype temp), TYPE &INPUT($*ltype temp)
%{ temp = ($*ltype)lua_tointeger(L,$input);
   $1 = &temp; %}
%typemap(in, numinputs=0) TYPE *OUTPUT ($*ltype temp)
%{ $1 = &temp; %}
%typemap(argout) TYPE *OUTPUT
%{  lua_pushinteger(L, (lua_Integer) *$1); SWIG_arg++;%}
%typemap(in) TYPE *INOUT = TYPE *INPUT;
%typemap(argout) TYPE *INOUT = TYPE *OUTPUT;
%typemap(in) TYPE &OUTPUT = TYPE *OUTPUT;
%typemap(argout) TYPE &OUTPUT = TYPE *OUTPUT;
%typemap(in) TYPE &INOUT = TYPE *INPUT;
%typemap(argout) TYPE &INOUT = TYPE *OUTPUT;
%typemap(in,checkfn="lua_isinteger") const TYPE *INPUT($*ltype temp)
%{ temp = ($*ltype)lua_tointeger(L,$input);
   $1 = &temp; %}

%enddef // LLDB_NUMBER_TYPEMAP

LLDB_NUMBER_TYPEMAP(unsigned char);
LLDB_NUMBER_TYPEMAP(signed char);
LLDB_NUMBER_TYPEMAP(short);
LLDB_NUMBER_TYPEMAP(unsigned short);
LLDB_NUMBER_TYPEMAP(signed short);
LLDB_NUMBER_TYPEMAP(int);
LLDB_NUMBER_TYPEMAP(unsigned int);
LLDB_NUMBER_TYPEMAP(signed int);
LLDB_NUMBER_TYPEMAP(long);
LLDB_NUMBER_TYPEMAP(unsigned long);
LLDB_NUMBER_TYPEMAP(signed long);

struct dvdrip_t {
  std::string file;
  std::string separator;
  std::string device;
  std::string path;
  std::string lua;
  unsigned int index;
};

struct title_t {
  unsigned int title_nr;
  unsigned int nr_chapters;
  unsigned int duration;
  unsigned int title_set_nr;
  unsigned int vts_ttn;
};

namespace std {
  %template(title_v) vector<title_t*>;
}

struct playback_time_t {
  unsigned int hour;
  unsigned int minute;
  unsigned int second;
  unsigned int usec;
};

int dvdrip_read_title(dvdrip_t*, title_t*, const char*);

%{
  lua_State* dvdrip_lua_init(dvdrip_t *dvdrip)
  {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (!dvdrip->lua.empty())
      {
	std::cout << dvdrip->lua << "\n";

	luaL_dofile(L, dvdrip->lua.c_str());

	/* This would normally be called by lua with the require function */
	/* Returns two tables, the global table and the module table */
	
	luaopen_dvd(L);
	
	if (lua_getglobal(L, "check_params") == LUA_TFUNCTION)
	  {
	    swig_type_info *info = SWIG_TypeQuery("dvdrip_t *");
	    assert(info);
	    SWIG_NewPointerObj(L, dvdrip, info, 0);
	    lua_call(L, 1, 1);
	    if (!lua_isboolean(L, lua_gettop(L)))
	      {
		fprintf(stderr, "Function check_params must return a boolean\n");
		exit(1);
	      }
	    int result = lua_toboolean(L, lua_gettop(L));
	    if (result == 0)
	      {
		fprintf(stderr, "Function check_params returned false, exiting\n");
		exit(1);
	      }
	  }
	else
	  lua_pop(L, 1);
      }
    return L;
  }

  int dvdrip_lua_titles(lua_State *L, dvdrip_t* dvdrip, title_v& titles)
  {
    if (lua_getglobal(L, "process_titles") == LUA_TFUNCTION)
      {
	SWIG_NewPointerObj(L, dvdrip, SWIG_TypeQuery("dvdrip_t *"), 0);
	SWIG_NewPointerObj(L, &titles, SWIG_TypeQuery("std::vector< title_t * > *"), 0);
	lua_call(L, 2, 0);
      }
    else
      lua_pop(L, 1);
    exit(1);
    return 0;
  }
%}


 
/* Local Variables:  */
/* mode: c           */
/* comment-column: 0 */
/* End:              */
