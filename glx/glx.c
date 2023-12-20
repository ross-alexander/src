/* ----------------------------------------------------------------------
--
-- 2023-12-20: Fix gcc warnings
--
---------------------------------------------------------------------- */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "mygl.h"

int debug_level = 0;

/* ----------------------------------------------------------------------
--
-- load_mygl
--
---------------------------------------------------------------------- */

struct mygl *load_mygl(const char *file)
{
  void *handle;

  struct mygl *mygl = calloc(sizeof(struct mygl), 1);

  /* --------------------
     Call dlerror to clear any errors
     -------------------- */

  dlerror();

  /* --------------------
     Open shared library with only local symbol export and lazy binding
     -------------------- */

  handle = dlopen(file, RTLD_LAZY|RTLD_LOCAL);
  char *error;
  if ((error = dlerror()) != 0)
    {
      printf("%s\n", error);
      exit(1);
    }
  printf("Using dynamic library %s\n", file);

  /* --------------------
     Get dynamic functions
     -------------------- */
  
  mygl->init = dlsym(handle, "init");
  mygl->reshape = dlsym(handle, "reshape");
  mygl->display = dlsym(handle, "display");

  assert(mygl->init);
  assert(mygl->reshape);
  assert(mygl->display);

  return mygl;
}

/* ----------------------------------------------------------------------
--
-- mygl_debug
--
---------------------------------------------------------------------- */

void mygl_debug(int level, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  if (debug_level >= level)
    vfprintf(stderr, format, ap);
  va_end(ap);
}

  
/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  int width = 1000;
  int height = 800;

  /* --------------------
     Load display dynamic library
     -------------------- */

  struct mygl *mygl = load_mygl((argc > 1) ? argv[1] : "./mygl.so");
  assert(mygl != 0);

  /* --------------------
     Open X Display
     -------------------- */

  Display *dpy = XOpenDisplay(0);
  if (dpy == 0)
    {
      fprintf(stderr, "Cannot open display\n");
      exit(1);
    }

  int screen = XDefaultScreen(dpy);
  Window root = RootWindow(dpy, screen);

  /* --------------------
     Get GLX details
     -------------------- */

  printf("%s %s\n", glXGetClientString(dpy, GLX_VENDOR), glXGetClientString(dpy, GLX_VERSION));

  /* --------------------
     Get a RGBA visual
     -------------------- */

  int vcount;
  XVisualInfo vinfo, *vinfos;
  vinfo.depth = 24;
  vinfo.class = TrueColor;
  int mask = VisualDepthMask | VisualClassMask;
  vinfos = XGetVisualInfo(dpy, mask, &vinfo, &vcount);

  if (vcount == 0)
    {
      printf("No 32 bit visual available\n");
      exit(1);
    }

  //  for (int i = 0; i < vcount; i++)
  //    printf("%d 0x%03x %d %d\n", i, vinfos[i].visualid, vinfos[i].depth, vinfos[i].bits_per_rgb);
  
  /* --------------------
     Find GLX frame buffer
     -------------------- */

  int count;
  int result;
  int attrs[] =
    {
      GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
      GLX_X_RENDERABLE, 1,
      GLX_RENDER_TYPE, GLX_RGBA_BIT,
      GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
      GLX_DOUBLEBUFFER, 1,
      GLX_RED_SIZE, 8,
      GLX_GREEN_SIZE, 8,
      GLX_BLUE_SIZE, 8,
      GLX_ALPHA_SIZE, 8,
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

  XVisualInfo *visinfo = 0;
  for (int j = 0; j < count; j++)
    {
      GLXFBConfig t = fbconfig[j];
      int gl_vid;
      glXGetFBConfigAttrib(dpy, t, GLX_VISUAL_ID, (int*)&gl_vid);
      
      for (int k = 0; k < vcount; k++)
	{
	  VisualID x_vid = vinfos[k].visualid;
	  if (x_vid == gl_vid)
	    {
	      visinfo = &vinfos[k];
	    }
	}
      if (visinfo)
	break;
    }
  assert(visinfo);
  printf("Found visual ID 0x%02x\n", visinfo->visualid);

  GLXContext cx = glXCreateContext(dpy, visinfo, 0, GL_TRUE); 

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
     Create X window with white background and border
     -------------------- */

  XSetWindowAttributes win_attrs;
  unsigned long background = XWhitePixel(dpy, screen);
  unsigned long border = XWhitePixel(dpy, screen);
  win_attrs.background_pixel = background;
  win_attrs.border_pixel = border;
  win_attrs.colormap = cm;

  /* --------------------
     Create a windows offset 10,10 from the root (WM will probably reparent and move window) and a 5 pixel border.
     -------------------- */

  Window win = XCreateWindow(dpy, root, 10, 10,
			     width, height, 5,
			     visinfo->depth,
			     InputOutput,
			     visinfo->visual,
			     CWColormap | CWBackPixel | CWBorderPixel,
			     &win_attrs);

  assert(win);

  /* --------------------
     Set inputs to Exposure (redraw), button press and strctureNotify (reshape)
     -------------------- */

  XSelectInput(dpy, win, ExposureMask | ButtonPressMask | StructureNotifyMask);
  XMapWindow(dpy, win);

  /* --------------------
     Set atoms to say we are not interested in Window Manager
     messages, including when the close window is clicked.
     -------------------- */

  Atom wm_protocols, wm_delete_window;
  wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
  wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(dpy, win, &wm_delete_window, 1);

  /* --------------------
     Run init first, but remember the window will not be mapped yet
     -------------------- */

  glXMakeCurrent(dpy, win, cx);
  if (mygl->init)
    mygl->init();

  /* --------------------
     Start X11 event loop
     -------------------- */

  while(1)
    {

      /* --------------------
	 If event available
	 -------------------- */

      if (XPending(dpy) > 0)
	{
	  XEvent event;
	  XNextEvent(dpy, &event);

	  switch(event.type)
	    {
	    case ClientMessage:
	      if (event.xclient.message_type == wm_protocols && event.xclient.data.l[0] == wm_delete_window)
		{
		  XCloseDisplay(dpy);
		  exit(0);
		}
	      break;
	    case ConfigureNotify:
	      {
		printf("ConfigureNotify\n");
		glXMakeCurrent(dpy, win, cx);
		if (mygl->reshape) mygl->reshape(event.xconfigure.width, event.xconfigure.height);
		break;
	      }
	    case NoExpose:
	      printf("NoExpose\n");
	      break;
	    case MapNotify:
	      printf("MapNotify\n");
	      mygl->mapped = 1;
	      break;
	    case ReparentNotify:
	      printf("Reparent\n");
	      break;
	    case Expose:
	      printf("Expose\n");
	      if (mygl->mapped && mygl->display)
		{
		  /* connect the context to the window */
		  glXMakeCurrent(dpy, win, cx);
		  /* clear the buffer */
		  mygl->display();
		  glFlush();
		  if (swap_flag) glXSwapBuffers(dpy,win); 
		  break;
		}
	    }
	}
      else
	{
	  if (mygl->mapped && mygl->display)
	    {
	      mygl->display();
	      glFlush();
	      if (swap_flag) glXSwapBuffers(dpy,win); 
	    }
	  usleep(100000);
	}
    }
}
