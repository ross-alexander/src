#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <cairo/cairo.h>

#include "common.h"

Visual* get_visual(Display *dpy, xtk_t* xtk)
{
  int mask, count, depth;
  XVisualInfo vinfo, *vinfos;
  Visual *visual;

  if (xtk->visual)
    {
      mask = VisualIDMask;
      vinfo.visualid = xtk->visual;
      vinfos = XGetVisualInfo(dpy, mask, &vinfo, &count);
      depth = vinfos[0].depth;
      visual = vinfos[0].visual;
      XFree(vinfos);
    }
  else
    {
      mask = VisualDepthMask | VisualClassMask;
      vinfo.depth = 32;
      vinfo.class = TrueColor;
      vinfos = XGetVisualInfo(dpy, mask, &vinfo, &count);
      //      printf("Found %d %d-bit visuals.\n", count, vinfo.depth);
      if (count > 0)
	{
	  depth = vinfos[0].depth;
	  visual = vinfos[0].visual;
	}
      XFree(vinfos);
      if (count == 0)
	{
	  mask = VisualDepthMask | VisualClassMask;
	  vinfo.depth = 24;
	  vinfo.class = TrueColor;
	  vinfos = XGetVisualInfo(dpy, mask, &vinfo, &count);
	  //	  printf("Found %d %d-bit visuals.\n", count, vinfo.depth);
	  if (count > 0)
	    {
	      depth = vinfos[0].depth;
	      visual = vinfos[0].visual;
	    }
	  XFree(vinfos);
	  if (count == 0)
	    {
	      return NULL;
	    }
	}
    }
  
  printf("Visual bits: %d\n", visual->bits_per_rgb);
  printf("Visual depth: %d\n", depth);
  printf("Visual ID: 0x%x\n", (int)visual->visualid);

  xtk->depth = vinfo.depth;
  xtk->visual = visual->visualid;
  return visual;
}

