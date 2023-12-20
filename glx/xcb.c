#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <X11/Xlib.h>

    /*
        If you get linking errors when using C++, you need
        to add extern "C" here or in X11-xcb.h, unless
        this bug is already fixed in your version:
        http://bugs.freedesktop.org/show_bug.cgi?id=22252
    */

#include <X11/Xlib-xcb.h> /* for XGetXCBConnection, link with libX11-xcb */

#include <xcb/xcb.h>
#include <xcb/glx.h>

#include <GL/glx.h>
#include <GL/gl.h>

#include "mygl.h"

int xcb_main_loop(xcb_connection_t *connection, xcb_glx_context_tag_t tag, xcb_window_t window, xcb_drawable_t drawable)
{
  int running = 1;

  int width;
  int height;

  printf("%d %d\n", window, drawable);

  while(running)
    {
      /* Wait for event */
      xcb_generic_event_t *event = xcb_wait_for_event(connection);
      if(!event)
	{
	  fprintf(stderr, "i/o error in xcb_wait_for_event");
	  return -1;
	}

      printf("Got event\n");

      switch(event->response_type & ~0x80)
	{
	case XCB_KEY_PRESS:
	  /* Quit on key press */
	  running = 0;
	  break;
	case XCB_MAP_NOTIFY:
	  printf("Map Notify\n");
	  //	  init();
	case XCB_CONFIGURE_NOTIFY:
	  {
	    printf("Configure Notify\n");
	    xcb_configure_notify_event_t *notify = (xcb_configure_notify_event_t*)event;
	    printf("%d %d\n", notify->width, notify->height);
	    //	    reshape(notify->width, notify->height);
	    break;
	  }
	case XCB_EXPOSE:
	  /* Handle expose event, draw and swap buffers */
	  printf("Expose\n");
	  //	  reshape(150, 150);
	  //	  display();
	  //	  xcb_glx_swap_buffers(connection, tag, drawable);
	  break;
	default:
	  break;
	}
      
      free(event);
    }
  
  return 0;
}

int main_loop(Display *xdisplay, xcb_connection_t *connection, xcb_window_t window, GLXDrawable drawable)
{
  int running = 1;
  while(running)
    {
      /* Wait for event */
      xcb_generic_event_t *event = xcb_wait_for_event(connection);
      if(!event)
	{
	  fprintf(stderr, "i/o error in xcb_wait_for_event");
	  return -1;
	}
      
      switch(event->response_type & ~0x80)
	{
	case XCB_KEY_PRESS:
	  /* Quit on key press */
	  running = 0;
	  break;
	case XCB_MAP_NOTIFY:
	  printf("Map Notify\n");
	  init();
	case XCB_CONFIGURE_NOTIFY:
	  {
	    printf("Configure Notify\n");
	    xcb_configure_notify_event_t *notify = (xcb_configure_notify_event_t*)event;
	    printf("%d %d\n", notify->width, notify->height);
	    reshape(notify->width, notify->height);
	    break;
	  }
	case XCB_EXPOSE:
	  /* Handle expose event, draw and swap buffers */
	  printf("Expose\n");
	  reshape(150, 150);
	  display();
	  glXSwapBuffers(xdisplay, drawable);
	  break;
	default:
	  break;
	}
      
      free(event);
    }
  
  return 0;
}

int setup_and_run(Display* display, xcb_connection_t *connection, int default_screen, xcb_screen_t *screen)
{
  int visualID = 0;
  
  /* Query framebuffer configurations */
  GLXFBConfig *fb_configs = 0;
  int num_fb_configs = 0;
  uint32_t fbconfig = 0;

  xcb_glx_get_fb_configs_cookie_t cookie = xcb_glx_get_fb_configs(connection, default_screen);
  xcb_glx_get_fb_configs_reply_t* reply = xcb_glx_get_fb_configs_reply(connection, cookie, 0);
  printf("Number of configs: %d %d %d\n", reply->num_FB_configs, reply->num_properties, xcb_glx_get_fb_configs_property_list_length(reply));

  uint32_t *list = xcb_glx_get_fb_configs_property_list(reply);
  for (int j = 0; j < reply->num_properties; j++)
    {
      if (list[2*j] == GLX_VISUAL_ID)
	{
	  visualID = list[2*j+1];
	  printf("Visual ID: %d\n", visualID);
	}
    }

#ifdef XLIB

  fb_configs = glXGetFBConfigs(display, default_screen, &num_fb_configs);
  printf("Number of configs: %d\n", num_fb_configs);

  if(!fb_configs || num_fb_configs == 0)
    {
      fprintf(stderr, "glXGetFBConfigs failed\n");
      return -1;
    }
  
  /* Select first framebuffer config and query visualID */
  GLXFBConfig fb_config = fb_configs[0];
  glXGetFBConfigAttrib(display, fb_config, GLX_VISUAL_ID , &visualID);
  printf("Visual ID: %d\n", visualID);
  
  GLXContext context;
  
  /* Create OpenGL context */
  context = glXCreateNewContext(display, fb_config, GLX_RGBA_TYPE, 0, True);
  if(!context)
    {
      fprintf(stderr, "glXCreateNewContext failed\n");
      return -1;
    }
#endif
  
  /* Create XID's for colormap and window */

  xcb_colormap_t colormap = xcb_generate_id(connection);
  xcb_window_t window = xcb_generate_id(connection);
  
  /* Create colormap */
  xcb_create_colormap(connection, XCB_COLORMAP_ALLOC_NONE, colormap, screen->root, visualID);
  
  /* Create window */
  uint32_t eventmask = XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;
  uint32_t valuelist[] = { eventmask, colormap, 0 };
  uint32_t valuemask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;
  
  xcb_create_window(connection,
		    XCB_COPY_FROM_PARENT,
		    window,
		    screen->root,
		    0, 0, 150, 150,
		    0,
		    XCB_WINDOW_CLASS_INPUT_OUTPUT,
		    visualID,
		    valuemask,
		    valuelist);
  
  // NOTE: window must be mapped before glXMakeContextCurrent
  xcb_map_window(connection, window);
  xcb_flush(connection);

#ifdef XLIB
  /* Create GLX Window */
  GLXDrawable drawable = 0;
  GLXWindow glxwindow =  glXCreateWindow(display, fb_config, window, 0);
#endif

  xcb_glx_window_t glxwindow = xcb_generate_id(connection);
  xcb_glx_create_window(connection, default_screen, fbconfig, window, glxwindow, 0, 0);
  xcb_flush(connection);
  if(!window)
    {
      xcb_destroy_window(connection, window);
#ifdef XCBXLIB
      glXDestroyContext(display, context);
#endif      
      fprintf(stderr, "glXDestroyContext failed\n");
      return -1;
    }
#ifdef XLIB
  drawable = glxwindow;
  /* make OpenGL context current */
  if(!glXMakeContextCurrent(display, drawable, drawable, context))
    {
      xcb_destroy_window(connection, window);
      glXDestroyContext(display, context);
      fprintf(stderr, "glXMakeContextCurrent failed\n");
      return -1;
    }
#endif

  xcb_glx_context_tag_t old, tag;
  xcb_drawable_t drawable = window;
  
  xcb_glx_context_t context = xcb_generate_id(connection);
  int direct = 0;
  int shared = 0;
  xcb_glx_create_new_context(connection, context, fbconfig, default_screen, GLX_RGBA_TYPE, shared, direct);
  xcb_flush(connection);

  xcb_generic_error_t* error;

  drawable = glxwindow;
  //  xcb_glx_context_tag_t old;
  //  xcb_glx_make_context_current_cookie_t cookie_ctx = xcb_glx_make_context_current(connection, 0, glxwindow, glxwindow, context);
  //  xcb_glx_make_context_current_reply_t *reply_ctx = xcb_glx_make_context_current_reply(connection, cookie_ctx, &error);

  xcb_glx_make_current_reply_t *reply_ctx = xcb_glx_make_current_reply(connection, xcb_glx_make_current(connection, glxwindow, context, 0), &error);


  printf("error code: %d\n", error->error_code);

  assert(error->error_code == 0);
  tag = reply_ctx->context_tag;

  int retval = xcb_main_loop(connection, tag, window, drawable);

  /* run main loop */
#ifdef XLIB
  int retval = main_loop(display, connection, window, drawable);
  
  /* Cleanup */
  glXDestroyWindow(display, glxwindow);
#endif

  xcb_destroy_window(connection, window);
#ifdef Foo  
  glXDestroyContext(display, context);
#endif

  return retval;
}

int main(int argc, char* argv[])
{
  Display *display;
  int default_screen;
  
  /* Open Xlib Display */ 
  display = XOpenDisplay(0);
  if(!display)
    {
      fprintf(stderr, "Can't open display\n");
      return -1;
    }
  
  default_screen = DefaultScreen(display);
  
  /* Get the XCB connection from the display */
  xcb_connection_t *connection =  XGetXCBConnection(display);
  if(!connection)
    {
      XCloseDisplay(display);
      fprintf(stderr, "Can't get xcb connection from display\n");
      return -1;
    }
  
  /* Acquire event queue ownership */
  XSetEventQueueOwner(display, XCBOwnsEventQueue);
  
  /* Find XCB screen */
  xcb_screen_t *screen = 0;
  xcb_screen_iterator_t screen_iter =  xcb_setup_roots_iterator(xcb_get_setup(connection));
  for(int screen_num = default_screen; screen_iter.rem && screen_num > 0; --screen_num, xcb_screen_next(&screen_iter));
  screen = screen_iter.data;
  
  /* Initialize window and OpenGL context, run main loop and deinitialize */  
  int retval = setup_and_run(display, connection, default_screen, screen);
  
  /* Cleanup */
  XCloseDisplay(display);
    return retval;
}
