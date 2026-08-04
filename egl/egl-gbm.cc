/* ----------------------------------------------------------------------
--
-- Check if EGL supports GBM
--
-- 2022-01-24
--
---------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

#define EGL_EGLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>

typedef std::vector<std::string> svec;

svec PrintExtensions(svec exts)
{
  int column = 0;
  for (auto &s: exts)
    {
      int len = s.length();
      if (column > 0 && column + len + 1 > 70)
	{
	  std::cout << "\n";
	  column = 0;
	}
       if (column == 0)
	 printf("    ");
       else
	 printf(" ");
       column += len + 1;
       std::cout << s;
    }
  if (column > 0) std::cout << "\n";
  return exts;
}


svec PrintDisplayExtensions(EGLDisplay d)
{
   svec exts;

   const char *extensions = eglQueryString(d, EGL_EXTENSIONS);
   if (!extensions)
      return exts;

#ifdef EGL_MESA_query_driver
   if (strstr(extensions, "EGL_MESA_query_driver")) {
      PFNEGLGETDISPLAYDRIVERNAMEPROC getDisplayDriverName =
         (PFNEGLGETDISPLAYDRIVERNAMEPROC)
            eglGetProcAddress("eglGetDisplayDriverName");
      printf("EGL driver name: %s\n", getDisplayDriverName(d));
   }
#endif

   /* Use boost library to do the split into a vector */
   
   boost::split(exts, std::string(extensions), boost::is_any_of(" "));
   
   puts(d == EGL_NO_DISPLAY ? "EGL client extensions string:" : "EGL extensions string:");

   return PrintExtensions(exts);
}

int main(int argc, char *argv[])
{
  svec clientext = PrintDisplayExtensions(EGL_NO_DISPLAY);
  std::map<std::string,int> extmap;
  for (auto &e: clientext)
    extmap[e] = 1;

  if(!extmap.contains("EGL_EXT_platform_base"))
    {
      std::cerr << "Requires EGL_EXT_platform_base\n";
    }

  PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

  if (!(extmap.contains("EGL_MESA_platform_gbm") || (extmap.contains("EGL_KHR_platform_gbm"))))
    { 
      std::cerr << "GBM platform missing\n";
      exit(1);
    }

  EGLDisplay display = getPlatformDisplay(EGL_PLATFORM_GBM_MESA,
					  EGL_DEFAULT_DISPLAY,
					  NULL);
  
  return(0);
}
