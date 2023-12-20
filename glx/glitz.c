#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <glitz.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#include <cairo-xlib.h>

int main()
{
  Display *dpy = XOpenDisplay(NULL);

  assert(dpy);

  int screen = XDefaultScreen(dpy);
  Window root = RootWindow(dpy, screen);

  printf("%s %s\n",
	 glXGetClientString(dpy, GLX_VENDOR),
	 glXGetClientString(dpy, GLX_VERSION));

   XSetWindowAttributes attr;
   unsigned long mask = 0;
   GLXContext ctx;
   XVisualInfo vinfo, *xvisinfo, *gvisinfo;
   
   /* --------------------
      X11 Visuals
      -------------------- */

   int count;
   //   vinfo.visualid = DefaultVisual(dpy, screen)->visualid;
   vinfo.screen = screen;
   vinfo.depth = 32;
   mask |= VisualDepthMask | VisualScreenMask;
   xvisinfo = XGetVisualInfo(dpy, mask, &vinfo, &count);

   for (int i = 0; i < count; i++)
     {
       if (xvisinfo[i].class == TrueColor)
	 printf("%02x TrueColor %d\n", xvisinfo[i].visualid, xvisinfo[i].depth);
       if (xvisinfo[i].class == DirectColor)
	 printf("%02x DirectColor %d\n", xvisinfo[i].visualid, xvisinfo[i].depth);
     }

   /* --------------------
      Get glitz format
      -------------------- */

   glitz_drawable_format_t* format = NULL;
   glitz_drawable_format_t  tmpl;
   mask = 0;
   tmpl.doublebuffer = 1;
   mask |= GLITZ_FORMAT_DOUBLEBUFFER_MASK;
   tmpl.samples = 1;
   mask |= GLITZ_FORMAT_SAMPLES_MASK;
   tmpl.color.fourcc = GLITZ_FOURCC_RGB;
   mask |= GLITZ_FORMAT_FOURCC_MASK;
   tmpl.color.red_size = tmpl.color.green_size = tmpl.color.blue_size = tmpl.color.alpha_size = 8;
   mask |= GLITZ_FORMAT_RED_SIZE_MASK;
   mask |= GLITZ_FORMAT_GREEN_SIZE_MASK;
   mask |= GLITZ_FORMAT_BLUE_SIZE_MASK;
   mask |= GLITZ_FORMAT_ALPHA_SIZE_MASK;

   //   format = glitz_glx_find_window_format(dpy, screen, mask, &tmpl, 0);
   //   assert(format);

   int index = 0;
   do
     {
       format = glitz_glx_find_window_format (dpy, screen, mask, &tmpl, index++);
       if (format)
	 {
 	   gvisinfo = glitz_glx_get_visual_info_from_format(dpy, screen, format);
	   printf("** %d %d %x %d\n", format->id, format->depth_size, gvisinfo->visualid, gvisinfo->depth);
	   if (gvisinfo->visualid == 0x2c)
	     break;
	   
	 }
     } while (format != NULL);

   assert(gvisinfo);
   printf("Got %d-bit visual VisualID %d (0x%02x)\n", gvisinfo->depth, gvisinfo->visualid, gvisinfo->visualid);

#ifdef UseGLX
   int attrib[] = { GLX_RGBA,
		    GLX_RED_SIZE, 1,
		    GLX_GREEN_SIZE, 1,
		    GLX_BLUE_SIZE, 1,
		    GLX_ALPHA_SIZE, 1,
		    GLX_DOUBLEBUFFER,
		    None };

   visinfo = glXChooseVisual(dpy, screen, attrib);
   assert(visinfo);
   printf("Get %d-bit visual VisualID %d (0x%02x)\n", visinfo->depth, visinfo->visualid, visinfo->visualid);
#endif

   int width = 500, height = 500;
   unsigned long background = XWhitePixel(dpy, screen);
   unsigned long border = XWhitePixel(dpy, screen);

  Colormap cm = XCreateColormap(dpy, RootWindow(dpy, screen), gvisinfo->visual, AllocNone);

  XSetWindowAttributes attrs;
  attrs.background_pixel = background;
  attrs.border_pixel = border;
  attrs.colormap = cm;

  Window win = XCreateWindow(dpy, root, 10, 10,
			     width, height, 5,
			     gvisinfo->depth,
			     InputOutput,
			     gvisinfo->visual,
			     CWColormap | CWBackPixel | CWBorderPixel,
			     &attrs);
#ifdef UseGLX
  ctx = glXCreateContext(dpy, visinfo, NULL, True);
  assert(ctx);
  glXMakeCurrent(dpy, win, ctx);
  printf("Here\n");
#endif

  XSelectInput(dpy, win, ExposureMask | ButtonPressMask | StructureNotifyMask);
  XMapWindow(dpy, win);

  printf("Format = %d depth = %d\n", format->id, format->depth_size);
    
  glitz_drawable_t* draw = glitz_glx_create_drawable_for_window(dpy, screen, format, win, width, height);
  assert(draw);
  glitz_format_t *f = glitz_find_standard_format(draw, GLITZ_STANDARD_ARGB32);
  printf("Format = %d\n", f->id);

  glitz_surface_t *surface = glitz_surface_create(draw, f, width, height, 0, NULL);
  assert(surface); 

  glitz_surface_attach(surface, draw, GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);

  Atom wm_protocols, wm_delete_window;
  wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
  wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(dpy, win, &wm_delete_window, 1);

  cairo_surface_t *csurface = cairo_glitz_surface_create(surface);
  assert(csurface);
  cairo_t *cr = cairo_create(csurface);
  assert(cr);

  while(1)
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
	case NoExpose:
	  printf("NoExpose\n");
	  break;
	case MapNotify:
	  printf("MapNotify\n");
	  break;
	case ReparentNotify:
	  printf("Reparent\n");
	  break;
	case Expose:
	  printf("Expose\n");

	  //	  cairo_surface_t *xsurface = cairo_xlib_surface_create(dpy, win, gvisinfo->visual, width, height);

	  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	  cairo_set_source_rgb(cr, 1, 1, 1);
	  cairo_new_path(cr);
	  cairo_rectangle(cr, 0, 0, width/2, height/2);
	  cairo_fill(cr);
	  glitz_drawable_flush(draw);
	  //	  glitz_drawable_swap_buffers(draw);

	  //	  XCopyArea(dpy, PicturePixmap, win, gc, 0, 0, width, height, 0, 0);
	  break;
	}
    }
  cairo_destroy(cr);
  cairo_surface_destroy(csurface);
}

