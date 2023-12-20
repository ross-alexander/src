#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <xcb/glx.h>


// ------------------------------------------------------------------------------------------------
// This function searches for an @param prop_name in the @param property list of properties of size @param prop. Prop is property count and not buffer size.


uint32_t glx_get_property(const uint32_t* property, const uint props, uint32_t prop_name){
  uint i=0;
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
  //  xcb_generic_error_t* xerror;

  xcb_glx_get_fb_configs_reply_t* fbconfigs = xcb_glx_get_fb_configs_reply(connection, xcb_glx_get_fb_configs(connection, screen_num), NULL);
  uint32_t* prop = xcb_glx_get_fb_configs_property_list(fbconfigs);

  uint32_t* fbconfig_line   = prop;
  uint32_t  fbconfig_linesz = fbconfigs->num_properties * 2;

  for(uint i = 0 ; i < fbconfigs->num_FB_configs; i++)
    {  // for each fbconfig line
      uint good_fbconfig = 1;

    for (uint j = 0 ;j < attribsz*2; j += 2)
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
        uint32_t fbconfig_id = glx_get_property(fbconfig_line, fbconfigs->num_properties , GLX_FBCONFIG_ID);
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
  if (fid_found)
    printf("glx_get_attrib_from_fbconfig: no attrib %u was found in a fbconfig with GLX_FBCONFIG_ID %u\n", attrib, fid);
  else
    printf("glx_get_attrib_from_fbconfig: GLX_FBCONFIG_ID %u was not found!\n", fid);
  return -1;
}

static int AttributeList[] = { GLX_RGBA, 0 };

xcb_glx_context_t glxcontext;
xcb_colormap_t    xcolormap;
xcb_window_t      xwindow;
xcb_glx_window_t  glxwindow;
xcb_glx_context_tag_t glxcontext_tag;
int32_t screen_number;
xcb_connection_t* xconnection;
xcb_screen_t* xscreen;

void setup_glx13()
{
  /* Find a FBConfig that uses RGBA.  Note that no attribute list is */
  /* needed since GLX_RGBA_BIT is a default attribute.*/
  uint32_t glxattribs[] = {
    GLX_DOUBLEBUFFER, 1,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_STENCIL_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_BUFFER_SIZE, 32,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT |
    GLX_PIXMAP_BIT | GLX_PBUFFER_BIT,
    GLX_X_RENDERABLE, 1
  };

  xcb_glx_fbconfig_t fbconfig = glx_choose_fbconfig( xconnection, screen_number, glxattribs, sizeof(glxattribs)/2/sizeof(uint32_t));
  xcb_visualid_t glxvisual = glx_get_attrib_from_fbconfig( xconnection, screen_number, fbconfig, GLX_VISUAL_ID );

	/* Create a GLX context using the first FBConfig in the list. */

  xcb_glx_create_new_context(xconnection, glxcontext, fbconfig, screen_number, GLX_RGBA_TYPE, 0, 1);

  assert(glxcontext);
  /* Create a colormap */
  xcb_create_colormap(xconnection, XCB_COLORMAP_ALLOC_NONE, xcolormap, xscreen->root, glxvisual );
  
  /* Create a window */
  uint32_t value_list[2], value_mask = XCB_CW_BACK_PIXEL | XCB_CW_COLORMAP ;
  value_list[0] = xscreen->white_pixel;
  value_list[1] = xcolormap;
  
  xcb_create_window(xconnection, xscreen->root_depth, xwindow,
		    xscreen->root, 0, 0, 1000, 1000, 0,
		    XCB_WINDOW_CLASS_INPUT_OUTPUT,
		    glxvisual,
		    value_mask,
		    value_list);
  xcb_map_window(xconnection, xwindow);

  /* Create a GLX window using the same FBConfig that we used for the */
  /* the GLX context.                                                 */

  xcb_glx_create_window( xconnection, screen_number, fbconfig, xwindow, glxwindow, 0, 0);

  /* Connect the context to the window for read and write */
  xcb_glx_make_context_current_cookie_t cookie_ctx =  xcb_glx_make_context_current( xconnection, 0, glxwindow, glxwindow, glxcontext );

  xcb_generic_error_t *error;

  xcb_glx_make_context_current_reply_t* reply_ctx = xcb_glx_make_context_current_reply ( xconnection, cookie_ctx, &error);

  if (error)
    printf("make_context_current %d\n", error->error_code);

  assert(reply_ctx != 0);
  glxcontext_tag = reply_ctx->context_tag;
}

/* ----------------------------------------------------------------------

   main

   ---------------------------------------------------------------------- */

int main(int argc, char **argv)
{
  xconnection = xcb_connect( NULL, &screen_number );
  
  // getting the default screen

  xscreen = xcb_setup_roots_iterator( xcb_get_setup( xconnection)).data;
  
  glxcontext = xcb_generate_id( xconnection );
  xcolormap  = xcb_generate_id( xconnection );
  xwindow    = xcb_generate_id( xconnection );
  glxwindow  = xcb_generate_id( xconnection );
  
  setup_glx13();
  xcb_flush(xconnection);

  /* clear the buffer */
  //  glClearColor(1,1,0,1);
  //  glClear(GL_COLOR_BUFFER_BIT);
  glLoadIdentity();
  glTranslatef(5, 5, 0);
  //  glRotatef(difftime(time(0), start), 0, 1.0f, 0.0f);
  //  glTranslatef(-5, -5, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glBegin (GL_TRIANGLES);
  glColor3f(1.0, 0.0, 0.0);
  glVertex2f(0.0, 0.0);
  glColor3f(0.0, 1.0, 0.0);
  glVertex2f(25.0, 0.0);
  glColor3f(0.0, 0.0, 1.0);
  glVertex2f(0.0, 25.0);
  glEnd();
  glFlush();
  
  // This kills xserver
  xcb_glx_finish_reply_t* reply = xcb_glx_finish_reply(xconnection,
						       xcb_glx_finish (xconnection, glxcontext_tag), 0);

  assert(reply);
  
  xcb_glx_swap_buffers( xconnection, glxcontext_tag, glxwindow );
  
  /* wait a while */
  sleep(4);
}
