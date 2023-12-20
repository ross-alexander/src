// gcc main2.c -o main -lxcb -lxcb-glx -lGL && ./main2
// TODO free replies

#include <stdio.h>
#include <stdlib.h>
#include <xcb/glx.h>
#include <GL/glx.h>
#include <GL/gl.h>

#define W 1024
#define H 1024

// parameter types returned by xcb_glx_get_fb_configs

#define GLX_DRAWABLE_TYPE               0x8010
#define GLX_RENDER_TYPE                 0x8011
#define GLX_DOUBLEBUFFER                5
#define GLX_RED_SIZE                    8
#define GLX_GREEN_SIZE                  9
#define GLX_BLUE_SIZE                   10
#define GLX_RGBA_BIT                    0x00000001
#define GLX_RGBA_TYPE                   0x8014
#define GLX_STENCIL_SIZE                13
#define GLX_DEPTH_SIZE                  12
#define GLX_BUFFER_SIZE                 2
#define GLX_ALPHA_SIZE                  11
#define GLX_X_RENDERABLE                0x8012

#define GLX_FBCONFIG_ID                 0x8013
#define GLX_VISUAL_ID                   0x800B

#define GLX_WINDOW_BIT                  0x00000001
#define GLX_PIXMAP_BIT                  0x00000002
#define GLX_PBUFFER_BIT                 0x00000004


// ------------------------------------------------------------------------------------------------
// fbconfig and visual?
uint32_t glx_attrs[] = {
  GLX_DOUBLEBUFFER, 1,
  GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT|GLX_PIXMAP_BIT|GLX_PBUFFER_BIT,
  GLX_X_RENDERABLE, 1,  
  GLX_RED_SIZE, 8,
  GLX_GREEN_SIZE, 8,
  GLX_BLUE_SIZE, 8,
  GLX_ALPHA_SIZE, 8,
  GLX_STENCIL_SIZE, 8,
  GLX_DEPTH_SIZE, 24,
  GLX_BUFFER_SIZE, 32,
  GLX_RENDER_TYPE, GLX_RGBA_BIT,
  GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
  //  GLX_SAMPLE_BUFFERS_ARB, 1,
  //  GLX_SAMPLES_ARB, 1,
  None
};

// ------------------------------------------------------------------------------------------------
// This function searches for an @param prop_name in the @param property list of properties of size @param prop. Prop is property count and not buffer size.
uint32_t glx_get_property(const uint32_t* property, const uint props, uint32_t prop_name)
{
  uint i = 0;
  while(i < props*2)
    {
      if(property[i] == prop_name)
        return property[i+1];
      else i += 2;
    }
  return -1;
}

// This function chooses and returns specific fbconfig id depending on attributes specified in 
// @param attrib list. @param attribsz is the number of properties(not list size)
int32_t glx_choose_fbconfig(xcb_connection_t* connection, uint32_t screen_num, uint32_t* attrib, uint32_t attribsz)
{
  xcb_generic_error_t* xerror;
  
  xcb_glx_get_fb_configs_reply_t* fbconfigs = xcb_glx_get_fb_configs_reply(connection, xcb_glx_get_fb_configs(connection, screen_num), NULL);
  uint32_t* prop = xcb_glx_get_fb_configs_property_list(fbconfigs);
  
  uint32_t* fbconfig_line   = prop;
  uint32_t  fbconfig_linesz = fbconfigs->num_properties * 2;
  
  for(uint i=0 ; i < fbconfigs->num_FB_configs; i++)
    {  // for each fbconfig line
      uint good_fbconfig = 1;
      
    for(uint j = 0; j < attribsz*2; j += 2)
      {  // for each attrib
	// if property found != property given
        if(glx_get_property(fbconfig_line, fbconfigs->num_properties, attrib[j]) != attrib[j+1])
	  {
            good_fbconfig = 0; // invalidate this fbconfig entry, sine one of the attribs doesn't match
            break;
	  }
      }
    
    // if all attribs matched, return with fid
    
    if(good_fbconfig)
      {
        uint32_t fbconfig_id = glx_get_property(fbconfig_line, fbconfigs->num_properties, GLX_FBCONFIG_ID);
        free(fbconfigs);
        return fbconfig_id;
      }

    fbconfig_line += fbconfig_linesz; // next fbconfig line;
  }
  return -1;
}

// This function returns @param attrib value from a line containing GLX_FBCONFIG_ID of @param fid
// It kind of queries particular fbconfig line for a specific property.
uint32_t glx_get_attrib_from_fbconfig(xcb_connection_t* connection, uint32_t screen_num, uint32_t fid, uint32_t attrib)
{
  xcb_glx_get_fb_configs_reply_t* fbconfigs = xcb_glx_get_fb_configs_reply(connection, xcb_glx_get_fb_configs(connection, screen_num), NULL);
  uint32_t* prop   = xcb_glx_get_fb_configs_property_list(fbconfigs);
  
  uint i = 0;
  uint fid_found = 0;
  while(i < fbconfigs->length)
    {
      if(prop[i] == GLX_FBCONFIG_ID)
	{
	  if(prop[i+1] == fid)
	    {
	      fid_found = 1;
	      i -= i%(fbconfigs->num_properties * 2); // going to start of the fbconfig  line
	      uint32_t attrib_value = glx_get_property(&prop[i], fbconfigs->num_properties, attrib);
	      free(fbconfigs);
	      return attrib_value;
	    }
	}
      i+=2;
    }
  if(fid_found)
    printf("glx_get_attrib_from_fbconfig: no attrib %u was found in a fbconfig with GLX_FBCONFIG_ID %u\n", attrib, fid);
  else
    printf("glx_get_attrib_from_fbconfig: GLX_FBCONFIG_ID %u was not found!\n", fid);
  return -1;
}

// ------------------------------------------------------------------------------------------------
int main()
{
  xcb_generic_error_t* xerror; // To hold errors!
  int screen_number;
  
  xcb_connection_t* connection = xcb_connect(NULL, &screen_number);
  xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;   // getting the default screen
  printf("screen %d  root %d\n", screen_number, screen->root);

  xcb_colormap_t    colormap    = xcb_generate_id(connection);  // generating XID's for our objects!
  xcb_window_t      window      = xcb_generate_id(connection);
  xcb_glx_context_t glx_context = xcb_generate_id(connection);
  xcb_glx_window_t  glx_window  = xcb_generate_id(connection);

  // ----------------------------------------------------------------------------------------------
  
  xcb_glx_query_version_reply_t* glx_version = xcb_glx_query_version_reply(connection, xcb_glx_query_version(connection, 0, 0), NULL);
  printf("glx %d.%d  response_type %x  pad0 %x  sequence %x  length %d\n",
	 glx_version->major_version, glx_version->minor_version, glx_version->response_type,
	 glx_version->pad0, glx_version->sequence, glx_version->length);
  
  // ----------------------------------------------------------------------------------------------
  
  xcb_glx_fbconfig_t fbconfig   = glx_choose_fbconfig(connection, screen_number, glx_attrs, sizeof(glx_attrs)/2/sizeof(uint32_t));
  xcb_visualid_t glx_visual     = glx_get_attrib_from_fbconfig(connection, screen_number, fbconfig, GLX_VISUAL_ID);
  printf("fbconfig %x glx_visual 0x%x\n", fbconfig, glx_visual);

  // ----------------------------------------------------------------------------------------------
  
  xcb_glx_create_new_context(connection, glx_context, fbconfig, screen_number, GLX_RGBA_TYPE, 0, 1);  // New-style context?
  
  // xcb_glx_create_context(connection, glx_context, glx_visual, 0, 0, 1);  // Alt method! Old-style context?

  if(!(xcb_glx_is_direct_reply(connection, xcb_glx_is_direct(connection, glx_context), NULL)->is_direct))
    puts("glx context is not direct!");

  // ----------------------------------------------------------------------------------------------
  
  xcb_create_colormap(connection , XCB_COLORMAP_ALLOC_NONE, colormap, screen->root, glx_visual);  // creating colormap

  // creating a window, using our new colormap
  
  uint32_t window_mask = XCB_CW_BACK_PIXEL|XCB_CW_EVENT_MASK|XCB_CW_COLORMAP;
  uint32_t window_attrs[] = {
    0xFF0000, // 0x444444,
    XCB_EVENT_MASK_EXPOSURE|XCB_EVENT_MASK_KEY_PRESS,
    colormap
  };
  
  xcb_create_window(connection, screen->root_depth, window, screen->root, 0,0, W,H, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, glx_visual, window_mask, window_attrs);
  xcb_map_window(connection, window);

  xcb_glx_create_window(connection, screen_number, fbconfig, window, glx_window, 0, NULL);
  xcb_flush(connection);

  // ----------------------------------------------------------------------------------------------
  
  xcb_glx_make_context_current_reply_t* reply_ctx = xcb_glx_make_context_current_reply(connection, xcb_glx_make_context_current(connection, 0, glx_window, glx_window, glx_context), NULL);
  if(!reply_ctx)
    puts("ERROR xcb_glx_make_context_current returned NULL!");
  
  xcb_glx_context_tag_t glx_context_tag = reply_ctx->context_tag;

  // alternative ?
  // xcb_glx_make_current_reply_t* reply_mc = xcb_glx_make_current_reply(connection, xcb_glx_make_current(connection, glx_window, glx_context, 0), NULL);
  // xcb_glx_context_tag_t glx_context_tag = reply_mc->context_tag;

  // ----------------------------------------------------------------------------------------------
  
  xcb_glx_get_error_reply(connection, xcb_glx_get_error(connection, glx_context_tag), &xerror);
  if(xerror)
    printf("\nERROR  xcb_glx_get_error %d\n", xerror->error_code);

  // ----------------------------------------------------------------------------------------------
  
  xcb_generic_event_t* event;
  uint running = 1;
  while(running)
    {
      event = xcb_poll_for_event(connection);
      if(event)
	{
	  switch (event->response_type)
	    {
	    case XCB_EXPOSE:
	  //          glClearColor(0, .5, 1, 1);  // Blue
	      glClearColor(1, 1, 1, 1);  // Blue
	      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	      glLoadIdentity();
	      glFlush();
	      xcb_glx_swap_buffers(connection, glx_context_tag, glx_window);
	      puts("Expose!");
	      break;
	    case XCB_KEY_PRESS: // exit on key press
	      running = 0;
	      break;
	    }
	}
      free(event);
    }
  xcb_disconnect(connection);
}
