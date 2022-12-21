#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <X11/Xlib.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <cairo/cairo-gl.h>

#include "common.h"
#include "pbutil.h"

int do_xtk(int argc, char *argv, int nwin, struct Xtk** xtk)
{
  struct Xtk *t = xtk[0];

  /* --------------------
     Open X Display
     -------------------- */

  Display *dpy = XOpenDisplay(0);
  XVisualInfo *visinfo;
  if (dpy == 0)
    {
      fprintf(stderr, "Cannot open display\n");
      exit(1);
    }

  int screen = XDefaultScreen(dpy);
  Window root = RootWindow(dpy, screen);

  int vcount;
  XVisualInfo vinfo, *vinfos;
  vinfo.depth = 32;
  vinfo.class = TrueColor;
  int mask = VisualDepthMask | VisualClassMask;
  vinfos = XGetVisualInfo(dpy, mask, &vinfo, &vcount);

  if (vcount == 0)
    {
      printf("No 32 bit visual available\n");
      exit(1);
    }

  /* --------------------
     Get GLX details
     -------------------- */

  printf("GLX: %s %s\n", glXGetClientString(dpy, GLX_VENDOR), glXGetClientString(dpy, GLX_VERSION));
 
  /* --------------------
     Find GLX frame buffer
     -------------------- */

  int count;
  int attrs[] =
    {
      GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
      GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
      GLX_X_RENDERABLE,	1,
      GLX_RENDER_TYPE,   GLX_RGBA_BIT,
      GLX_RED_SIZE,      8,
      GLX_GREEN_SIZE,    8,
      GLX_BLUE_SIZE,     8,
      GLX_ALPHA_SIZE,    8,
      GLX_DOUBLEBUFFER,  GL_TRUE,
      GLX_DEPTH_SIZE,    24,
      GLX_SAMPLE_BUFFERS_ARB, 1,
      GLX_SAMPLES_ARB, 1,
      None
  };

  GLXFBConfig *fbconfig = glXChooseFBConfig(dpy, screen, attrs, &count);

  if (count == 0)
    {
      fprintf(stderr, "No suitable framebuffer found\n");
      exit(1);
    }

  visinfo = 0;
  for (int j = 0; j < count; j++)
    {
      GLXFBConfig t = fbconfig[j];
      VisualID vid;
      glXGetFBConfigAttrib(dpy, t, GLX_VISUAL_ID, (int*)&vid);
      for (int k = 0; k < vcount; k++)
	{
	  if ((vinfos[k].visualid == vid))
	    visinfo = &vinfos[k];
	}
      if (visinfo)
	break;
    }
  assert(visinfo);

  /* --------------------
     Just use first suitable FB
     -------------------- */

  printf("GLX: Using X Visual 0x%x\n", (int)visinfo->visualid);

  /* --------------------
     Check if double buffering is on (should always be on since we ask
     for a doublebuffer config).
     -------------------- */
  
  int swap_flag;
  glXGetFBConfigAttrib(dpy, fbconfig[0], GLX_DOUBLEBUFFER, &swap_flag);

  /* --------------------
     Create X colormap
     -------------------- */

  Colormap cm = XCreateColormap(dpy, root, visinfo->visual, AllocNone);

  /* --------------------
     Create X window2
     -------------------- */

  XSetWindowAttributes win_attrs;
  unsigned long background = XWhitePixel(dpy, screen);
  unsigned long border = XWhitePixel(dpy, screen);
  win_attrs.background_pixel = 0x00000000;
  win_attrs.border_pixel = border;
  win_attrs.colormap = cm;

  Window win = XCreateWindow(dpy, root, 10, 10,
			     t->width, t->height, 5,
			     visinfo->depth,
			     InputOutput,
			     visinfo->visual,
			     CWColormap | CWBackPixel | CWBorderPixel,
			     &win_attrs);
  
  XSelectInput(dpy, win, ExposureMask | ButtonPressMask | StructureNotifyMask);
  XMapWindow(dpy, win);
  
  Atom wm_protocols, wm_delete_window;
  wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
  wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(dpy, win, &wm_delete_window, 1);

  /* --------------------
     create a GLX context
     -------------------- */

  GLXContext cx = glXCreateContext(dpy, visinfo, 0, GL_TRUE); 
  cairo_device_t *device = cairo_glx_device_create(dpy, cx);

  int mapped = 0;

  while(1)
    {
      XEvent event;
      XNextEvent(dpy, &event);
      /*
      Atom returntype;
      int returnformat;
      unsigned long nitems;
      unsigned long nbytes;
      int *prop_return;
      XGetWindowProperty(dpy, event.xexpose.window, xtkid, 0, 1, 0, XA_INTEGER, &returntype, &returnformat, &nitems, &nbytes, (unsigned char**)&prop_return);
      struct Xtk_xlib *t = (struct Xtk_xlib*)&xlib[prop_return[0]];
      XFree(prop_return);
      */

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
	    int w = event.xconfigure.width;
	    int h = event.xconfigure.height;
	    t->width = w;
	    t->height = h;
	    //	    printf("GLX-event: ConfigureNotify (%d x %d)\n", w, h);
	    break;
	  }
	case NoExpose:
	  printf("NoExpose\n");
	  break;
	case MapNotify:
	  printf("GLX-event: MapNotify\n");
	  mapped = 1;
	  t->surface = cairo_gl_surface_create_for_window (device, win, t->width, t->height);
	  break;
	case ReparentNotify:
	  printf("GLX-event: Reparent\n");
	  break;
	case Expose:
	  {
	    if (mapped)
	      {
		printf("GLX-event: Expose\n");
		//		glXMakeCurrent(dpy, win, cx);
		cairo_draw_surface(t);
		cairo_gl_surface_swapbuffers(t->surface);
	      }
	    break;
	  }
	default:
	  printf("Event type %d\n", event.type);
	  break;
	}
    }
  return 0;
}

const char *id() { return "glx"; }
