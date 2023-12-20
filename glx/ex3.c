// This example program creates two EGL surfaces: one from an X11 Window
// and the other from an X11 Pixmap.
//
// Compile with `cc example.c -lxcb -lEGL`.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// #define EGL_EGLEXT_PROTOTYPES

#include <GL/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <xcb/xcb.h>
#include <xcb/glx.h>

struct my_display {
  xcb_connection_t *x11;
  int screen;
  int root_of_screen;
  EGLDisplay egl;
};

struct my_config {
  struct my_display dpy;
  xcb_colormap_t colormap;
  xcb_visualid_t visualid;
  int depth;
  EGLConfig egl;
};

struct my_window {
  struct my_config config;
  xcb_window_t x11;
  EGLSurface egl;
};

struct my_pixmap {
  struct my_config config;
  xcb_pixmap_t x11;
  EGLSurface egl;
};

static void check_extensions(void) {
  const char *client_extensions =
    eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
  
  if (!client_extensions) {
    // EGL_EXT_client_extensions is unsupported.
    abort();
  }
  if (!strstr(client_extensions, "EGL_EXT_platform_xcb")) {
    abort();
  }
}

xcb_screen_t *get_screen(xcb_connection_t *c, int screen) {
  xcb_screen_iterator_t iter;
  
  iter = xcb_setup_roots_iterator(xcb_get_setup(c));
  for (; iter.rem; --screen, xcb_screen_next(&iter))
    if (screen == 0)
      return iter.data;
  
  return NULL;
}

int get_visual_depth(xcb_connection_t *c, xcb_visualid_t visual) {
  const xcb_setup_t *setup = xcb_get_setup(c);
  for (xcb_screen_iterator_t i = xcb_setup_roots_iterator(setup); i.rem;
       xcb_screen_next(&i)) {
    for (xcb_depth_iterator_t j =
	   xcb_screen_allowed_depths_iterator(i.data);
	 j.rem; xcb_depth_next(&j)) {
      const int len = xcb_depth_visuals_length(j.data);
      const xcb_visualtype_t *visuals = xcb_depth_visuals(j.data);
      for (int k = 0; k < len; k++) {
	if (visual == visuals[k].visual_id) {
	  return j.data->depth;
	}
      }
    }
  }
  abort();
}

static struct my_display get_display(void)
{
  struct my_display dpy;
  
  dpy.x11 = xcb_connect(NULL, &dpy.screen);
  if (!dpy.x11) {
    abort();
  }
  
  dpy.egl = eglGetPlatformDisplay(EGL_PLATFORM_XCB_EXT, dpy.x11,
				     (const EGLAttrib[]){
				       EGL_PLATFORM_XCB_SCREEN_EXT,
				       dpy.screen,
				       EGL_NONE,
				     });
  
  if (dpy.egl == EGL_NO_DISPLAY) {
    abort();
  }
  
  EGLint major, minor;
  if (!eglInitialize(dpy.egl, &major, &minor)) {
    abort();
  }
  
  xcb_screen_t *screen = get_screen(dpy.x11, dpy.screen);
  dpy.root_of_screen = screen->root;
  
  return dpy;
}

static struct my_config get_config(struct my_display dpy) {
  struct my_config config = {
    .dpy = dpy,
  };
  
  EGLint egl_config_attribs[] = {
    EGL_BUFFER_SIZE,
    32,
    EGL_RED_SIZE,
    8,
    EGL_GREEN_SIZE,
    8,
    EGL_BLUE_SIZE,
    8,
    EGL_ALPHA_SIZE,
    8,
    
    EGL_DEPTH_SIZE,
    EGL_DONT_CARE,
    EGL_STENCIL_SIZE,
    EGL_DONT_CARE,
    
    EGL_RENDERABLE_TYPE,
    EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE,
    EGL_WINDOW_BIT | EGL_PIXMAP_BIT,
    EGL_NONE,
  };
  
  EGLint num_configs;
  if (!eglChooseConfig(dpy.egl, egl_config_attribs, &config.egl, 1,
		       &num_configs)) {
    abort();
  }
  if (num_configs == 0) {
    abort();
  }
  
  if (!eglGetConfigAttrib(dpy.egl, config.egl, EGL_NATIVE_VISUAL_ID,
			  (EGLint *)&config.visualid)) {
    abort();
  }
  
  config.colormap = xcb_generate_id(dpy.x11);
  if (xcb_request_check(dpy.x11,
			xcb_create_colormap_checked(
						    dpy.x11, XCB_COLORMAP_ALLOC_NONE, config.colormap,
						    dpy.root_of_screen, config.visualid))) {
    abort();
  }
  
  config.depth = get_visual_depth(dpy.x11, config.visualid);
  
  return config;
}


static struct my_window get_window(struct my_config config)
{
  xcb_generic_error_t *e;
  
  struct my_window window = {
    .config = config,
  };
  
  window.x11 = xcb_generate_id(config.dpy.x11);
  e = xcb_request_check(
			config.dpy.x11,
			xcb_create_window_checked(config.dpy.x11,            // connection
						  XCB_COPY_FROM_PARENT,      // depth
						  window.x11,                // window id
						  config.dpy.root_of_screen, // root
						  0, 0,                      // x, y
						  1024, 1024,                  // width, height
						  0,                         // border_width
						  XCB_WINDOW_CLASS_INPUT_OUTPUT, // class
						  config.visualid,               // visual
						  XCB_CW_EVENT_MASK|XCB_CW_COLORMAP,               // mask
						  (const int[]){
						    XCB_EVENT_MASK_EXPOSURE|XCB_EVENT_MASK_KEY_PRESS,
						    config.colormap,
						    XCB_NONE,
						  }));
  if (e) {
    abort();
  }
  
  window.egl = eglCreatePlatformWindowSurface(config.dpy.egl, config.egl,
					      &window.x11, NULL);
  
  if (window.egl == EGL_NO_SURFACE)
    {
      abort();
    }
  return window;
}

static struct my_pixmap get_pixmap(struct my_config config) {
  struct my_pixmap pixmap = {
    .config = config,
  };
  
  pixmap.x11 = xcb_generate_id(config.dpy.x11);
  if (xcb_request_check(
			config.dpy.x11,
			xcb_create_pixmap(config.dpy.x11, config.depth, pixmap.x11,
					  config.dpy.root_of_screen, 256, 256))) {
    abort();
  }
  
  pixmap.egl = eglCreatePlatformPixmapSurface(config.dpy.egl, config.egl,
					      &pixmap.x11, NULL);
  
  if (pixmap.egl == EGL_NO_SURFACE)
    {
      abort();
    }
  return pixmap;
}

int main(void)
{
  check_extensions();
  
  struct my_display dpy = get_display();
  struct my_config config = get_config(dpy);
  struct my_window window = get_window(config);
  struct my_pixmap pixmap = get_pixmap(config);

  xcb_map_window(dpy.x11, window.x11);
  xcb_flush(dpy.x11);

  EGLint ctxattr[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };
  
  EGLContext egl_context = eglCreateContext(dpy.egl, config.egl, EGL_NO_CONTEXT, ctxattr );
    if ( egl_context == EGL_NO_CONTEXT ) {
        printf("CreateContext, EGL eglError: %d\n", eglGetError() );
        return 0;
    }
  
  //  xcb_glx_make_context_current_reply_t* reply_ctx = xcb_glx_make_context_current_reply(dpy.x11, xcb_glx_make_context_current(dpy.x11, 0, window.x11, window.x11, window.egl), NULL);
  //  if(!reply_ctx)
  //    puts("ERROR xcb_glx_make_context_current returned NULL!");
  
  //  xcb_glx_context_tag_t glx_context_tag = reply_ctx->context_tag;

  
  //  xcb_glx_get_error_reply(connection, xcb_glx_get_error(connection, glx_context_tag), &xerror);
  //  if(xerror)
  //    printf("\nERROR  xcb_glx_get_error %d\n", xerror->error_code);

  // ----------------------------------------------------------------------------------------------
  
  xcb_generic_event_t* event;
  uint running = 1;
  while(running)
    {
      event = xcb_poll_for_event(dpy.x11);
      if(event)
	{
	  switch (event->response_type)
	    {
	    case XCB_EXPOSE:
	      eglMakeCurrent(dpy.egl, window.egl, window.egl, egl_context);
	       glClearColor(0, .5, 1, 1);  // Blue
	      //	      glClearColor(1, 1, 1, 1);  // Blue
	       glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	      //	      glLoadIdentity();
	      //	      glFlush();
	      //	      xcb_glx_swap_buffers(connection, glx_context_tag, glx_window);
	       eglSwapBuffers(dpy.egl, window.egl);
	       puts("Expose!");
	      break;
	    case XCB_KEY_PRESS: // exit on key press
	      running = 0;
	      break;
	    }
	}
      free(event);
    }

  
  return 0;
}
