#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <xcb/xcb.h>
#include <xcb/render.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_renderutil.h>

#include <cairo-xcb.h>

#include "common.h"

typedef struct xtk_xcb_t {
  unsigned int depth;
  xcb_drawable_t window;
  xcb_visualtype_t* visual;
  xcb_colormap_t cmap;
  xtk_t *xtk;
  int mapped;
  int width;
  int height;
  cairo_surface_t *surface;
} xtk_xcb_t;

/* ----------------------------------------------------------------------
--
-- do_xcb_get_xtkid
--
---------------------------------------------------------------------- */
int do_xcb_get_xtkid(xcb_connection_t *c, xcb_window_t window, xcb_atom_t atom)
{
  xcb_get_property_cookie_t cookie = xcb_get_property(c, 0, window, atom, XCB_ATOM_INTEGER, 0, 1);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(c, cookie, NULL);
  int *r = xcb_get_property_value(reply);
  int id = r[0];
  free(reply);
  return id;
}


/* ----------------------------------------------------------------------
--
-- do_xcb
--
---------------------------------------------------------------------- */

int do_xtk(int argc, char *argv[], unsigned int nwin, xtk_t **xtk)
{
  int i, screenNum;

  /* --------------------
     count number of windows
     -------------------- */

  struct xtk_xcb_t *xtkxcb = calloc(sizeof(xtk_xcb_t), nwin);

  /* --------------------
     connect via xcb
     -------------------- */

  xcb_connection_t *connection = xcb_connect(0, &screenNum);

  /* Get the screen whose number is screenNum */
  
  const xcb_setup_t *setup = xcb_get_setup(connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);  
  
  // we want the screen at index screenNum of the iterator
  for (i = 0; i < screenNum; ++i)
    {
      xcb_screen_next(&iter);
    }

  xcb_screen_t *screen = iter.data;

  xcb_intern_atom_reply_t* atom = xcb_intern_atom_reply(connection, xcb_intern_atom(connection, 0, strlen("XTK_ID"), "XTK_ID"), 0);
  assert(atom->atom != 0);

  printf ("XCB: Informations of screen %ld:\n", (long)screen->root);
  printf ("XCB:   width.........: %d\n", screen->width_in_pixels);
  printf ("XCB:   height........: %d\n", screen->height_in_pixels);
  printf ("XCB:   white pixel...: %ld\n", (long)screen->white_pixel);
  printf ("XCB:   black pixel...: %ld\n", (long)screen->black_pixel);
  printf ("XCB:   root depth....: %ld\n", (long)screen->root_depth);
  printf ("XCB:   visual id.....: %ld\n", (long)screen->root_visual);
  printf ("\n");

  for (i = 0; i < nwin; i++)
    {
      struct xtk_xcb_t *t = &xtkxcb[i];
      t->xtk = xtk[i];
      t->mapped = 0;

      if (xtk[i]->visual)
	{
	  t->visual = xcb_aux_find_visual_by_id(screen, xtk[i]->visual);
	  assert(t->visual);
	}
      else if ((t->visual = xcb_aux_find_visual_by_attrs(screen, XCB_VISUAL_CLASS_TRUE_COLOR, 32)))
	{
	}
      else
	{
	  t->visual = xcb_aux_find_visual_by_id(screen, screen->root_visual);
	  assert(t->visual);
	  if (screen->root_depth != 24 || t->visual->_class != XCB_VISUAL_CLASS_TRUE_COLOR)
	    {
	      printf("Only 24 bit TrueColor visuals for now\n");
	      exit(1);
	    }
	}
      t->depth = xcb_aux_get_depth_of_visual(screen, t->visual->visual_id);

      printf ("  window depth..: %d\n", (int)t->depth);
      printf ("  visual id.....: 0x%x\n", (int)t->visual->visual_id);
      printf ("\n");
      
      t->cmap = xcb_generate_id(connection);
      xcb_create_colormap(connection, XCB_COLORMAP_ALLOC_NONE, t->cmap, screen->root, t->visual->visual_id);
      xcb_aux_sync(connection);
      
      xcb_params_cw_t params;
      params.colormap = t->cmap;
      params.back_pixel = 0;
      params.border_pixel = 0;
      params.event_mask = XCB_EVENT_MASK_EXPOSURE |XCB_EVENT_MASK_STRUCTURE_NOTIFY;
      uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_COLORMAP | XCB_CW_EVENT_MASK;

      t->width = t->xtk->width;
      t->height = t->xtk->height;

      t->window = xcb_generate_id(connection);
      xcb_aux_create_window_checked (connection,                    /* Connection          */
				     t->depth,                      /* depth (same as root)*/
				     t->window,                     /* window Id           */
				     screen->root,                  /* parent window       */
				     0, 0,                          /* x, y                */
				     t->width, t->height,           /* width, height       */
				     10,                            /* border_width        */
				     XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
				     t->visual->visual_id,          /* visual              */
				     mask, &params);                /* masks and params    */

      xcb_change_property(connection, XCB_PROP_MODE_REPLACE, t->window, atom->atom, XCB_ATOM_INTEGER, 32, 1, &i);
      xcb_map_window(connection, t->window);
      xcb_flush(connection);
    }

  /* --------------------
     xcb event loop
     -------------------- */

  xcb_generic_event_t* ev;
  while((ev = xcb_wait_for_event(connection)))
    {
      switch(XCB_EVENT_RESPONSE_TYPE(ev))
	{
	case XCB_EXPOSE:
	  {
	    //	    printf("XCB-event: Expose\n");
	    
	    xcb_expose_event_t *notify = (xcb_expose_event_t*)ev;
	    int id = do_xcb_get_xtkid(connection, notify->window, atom->atom);
	    struct xtk_xcb_t *x = xtkxcb + id;
	    if (x->mapped)
	      {
		xtk_draw_surface(x->xtk, x->surface);
		xcb_flush(connection);
	      }
	    break;
	  }
	case XCB_CONFIGURE_NOTIFY:
	  {
	    xcb_configure_notify_event_t *notify = (xcb_configure_notify_event_t*)ev;
	    int id = do_xcb_get_xtkid(connection, notify->window, atom->atom);
	    struct xtk_xcb_t *x = &xtkxcb[id];
	    if (x->mapped)
	      {
		x->xtk->width = x->width = notify->width;
		x->xtk->height = x->height = notify->height;
		printf("XCB-event: ConfigureNotify %d × %d\n", x->width, x->height);
		cairo_xcb_surface_set_size(x->surface, x->width, x->height);
	      }
	    break;
	  }
	case XCB_NO_EXPOSURE:
	  //	  printf("No exposure\n");
	  break;
	case XCB_MAP_NOTIFY:
	  {
	    printf("XCB-event: MapNotify\n");
	    xcb_map_notify_event_t *notify = (xcb_map_notify_event_t*)ev;
	    int id = do_xcb_get_xtkid(connection, notify->window, atom->atom);
	    struct xtk_xcb_t *x = xtkxcb + id;
	    x->mapped = 1;
	    x->surface = cairo_xcb_surface_create(connection, x->window, x->visual, x->width, x->height);
	    break;
	  }
	case XCB_REPARENT_NOTIFY:
	  //	  printf("XCB-event: ReparentNotify\n");
	  break;
	case XCB_RESIZE_REQUEST:
	  {
	    printf("XCB: Resize\n");
	    xcb_resize_request_event_t *resize = (xcb_resize_request_event_t*)ev;
	    int id = do_xcb_get_xtkid(connection, resize->window, atom->atom);
	    struct xtk_xcb_t *x =  xtkxcb + id;
	    x->width = resize->width;
	    x->height = resize->height;
	    uint32_t values[] = { x->width, x->height };
	    xcb_configure_window (connection, x->window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
	    xcb_flush(connection);
	    break;
	  }
	case 0:
	  {
	    xcb_generic_error_t *err = (xcb_generic_error_t *) ev;
	    printf("Error: %d after sequence %d\n", err->error_code, (unsigned int) err->full_sequence);
	    exit(1);
	    break;
	  }
	default:
	  printf("Unknown event %d.\n", ev->response_type & 0x7f);
	  break;
	}
      free(ev);
    }
  return 1;
}

const char *id(void)
{
  return "xcb";
}

