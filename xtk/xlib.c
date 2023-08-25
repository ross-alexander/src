/* ----------------------------------------------------------------------
--
-- xlib
--
-- Raw xlib
--
---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <cairo/cairo-xlib.h>

#include "common.h"
#include "visual.h"

struct xtk_xlib_t {
  Visual *visual;
  Colormap cmap;
  Drawable window;
  int mapped;
  int width;
  int height;
  cairo_surface_t *surface;
  xtk_t *xtk;
};


/* ----------------------------------------------------------------------
--
-- do_xlib
--
---------------------------------------------------------------------- */

int do_xtk(int argc, char *argv[], unsigned int nwin, xtk_t **xtk)
{
  Display *dpy = XOpenDisplay(0);
  if (dpy == 0)
    {
      fprintf(stderr, "%s:  unable to open display \"%s\"\n", argv[0], XDisplayName(":0"));
      exit(1);
    }

  int screen = XDefaultScreen(dpy);
  Drawable root = XRootWindow(dpy, screen);

  int rootx, rooty;
  unsigned int rootwidth, rootheight, rootbw, rootdepth;

  XGetGeometry(dpy, root, &root, &rootx, &rooty, &rootwidth, &rootheight, &rootbw, &rootdepth);
 
  printf("Xlib: Display width: %d\n", XDisplayWidth(dpy, screen));
  printf("Xlib: Display height: %d\n", XDisplayHeight(dpy, screen));
  printf("Xlib: Display depth: %d\n", rootdepth);
  printf("Xlib: Pad: %d\n", XBitmapPad(dpy));
  printf("Xlib: Black: %08lx\n", BlackPixel(dpy, screen));
  printf("Xlib: White: %08lx\n", WhitePixel(dpy, screen));

  unsigned long background = BlackPixel(dpy, screen);
  unsigned long border = BlackPixel(dpy, screen);

  Atom wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
  Atom wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

  /* --------------------
     Create atom XTK_ID to record xtk number
     -------------------- */

  Atom xtkid = XInternAtom(dpy, "XTK_ID", False);      

  struct xtk_xlib_t xlib[nwin];

  //  memset(xlib, 0, sizeof(struct xtk_xlib_t) * nwin);
  
  for (int i = 0; i < nwin; i++)
    {
      struct xtk_xlib_t *t = &xlib[i];

      t->xtk = xtk[i];
      t->mapped = 0;
      t->visual = get_visual(dpy, xtk[i]);

      printf("Xlib: Visual bits: %d\n", t->visual->bits_per_rgb);
      printf("Xlib: Visual depth: %d\n", t->xtk->depth);
      printf("Xlib: Visual ID: 0x%x\n", (int)t->visual->visualid);

      t->cmap = XCreateColormap(dpy, RootWindow(dpy, screen), t->visual, AllocNone);

      XSetWindowAttributes attrs;
      attrs.background_pixel = background;
      attrs.border_pixel = border;
      attrs.colormap = t->cmap;

      t->width = t->xtk->width;
      t->height = t->xtk->height;
      
      printf("xlib: %s [%d × %d]\n", t->xtk->title, t->width, t->height);
      
      /* --------------------
	 Create X window
	 -------------------- */
      
      t->window = XCreateWindow(dpy, RootWindow(dpy, screen), 10, 10, 
				t->width, t->height,
				5,
				t->xtk->depth,
				InputOutput,
				t->visual,
				CWColormap | CWBackPixel | CWBorderPixel, &attrs);
      
      XSelectInput(dpy, t->window, ExposureMask | ButtonPressMask | StructureNotifyMask);
      XMapWindow(dpy, t->window);
      
      XSetWMProtocols(dpy, t->window, &wm_delete_window, 1);

      /* --------------------
	 Set XTK_ID to array offset
	 -------------------- */

      XChangeProperty(dpy, t->window, xtkid, XA_INTEGER, 32, PropModeReplace, (unsigned char*)&i, 1);
    }

  while(1)
    {
      XEvent event;
      XNextEvent(dpy, &event);

      Atom returntype;
      int returnformat;
      unsigned long nitems;
      unsigned long nbytes;
      int *prop_return;
      XGetWindowProperty(dpy, event.xexpose.window, xtkid, 0, 1, 0, XA_INTEGER, &returntype, &returnformat, &nitems, &nbytes, (unsigned char**)&prop_return);

      //      printf("nitems %ld nbytes = %ld\n", nitems, nbytes);

      if (nitems)
	{      
	  struct xtk_xlib_t *t = (struct xtk_xlib_t*)&xlib[prop_return[0]];
	  XFree(prop_return);
	  
	  switch(event.type)
	    {
	    case ButtonPress:
	      break;
	    case ClientMessage:
	      if (event.xclient.message_type == wm_protocols && event.xclient.data.l[0] == wm_delete_window)
		{
		  XCloseDisplay(dpy);
		  exit(0);
		}
	      break;
	    case ResizeRequest:
	      printf("Xlib: Resize\n");
	      break;
	    case ConfigureNotify:
	      {
		t->xtk->width = t->width = event.xconfigure.width;
		t->xtk->height = t->height = event.xconfigure.height;
		if (t->mapped)
		  {
		    printf("Xlib-event: ConfigureNotify (%d × %d)\n", t->xtk->width, t->xtk->height);
		    //		    cairo_xlib_surface_set_size(t->surface, t->width, t->height);
		  }
		break;
	      }
	    case NoExpose:
	      printf("NoExpose\n");
	      break;
	    case MapNotify:
	      printf("Xlib-event: MapNotify\n");
	      t->mapped = 1;
	      //	      t->surface = cairo_xlib_surface_create(dpy, t->window, t->visual, t->width, t->height);
	      //	      assert(cairo_surface_status(t->surface) == CAIRO_STATUS_SUCCESS);
	      break;
	    case ReparentNotify:
	      printf("Xlib-event: Reparent\n");
	      break;
	    case Expose:
	      {
		printf("Xlib-event: Expose\n");
		if (t->mapped)
		  {
		    t->surface = cairo_xlib_surface_create(dpy, t->window, t->visual, t->width, t->height);
		    assert(cairo_surface_status(t->surface) == CAIRO_STATUS_SUCCESS);
		    xtk_draw_surface(t->xtk, t->surface);
		    cairo_surface_destroy(t->surface);
		  }
		break;
	      }
	    default:
	      printf("Event type %d\n", event.type);
	      break;
	    }
	}
    }
}

const char *id()
{
  return "xlib";
}
