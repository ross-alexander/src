#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_image.h>

#include <cairo.h>
#include <cairo-xcb.h>

#include <glib.h>

#include "mandel.h"

xcb_format_t *find_format(xcb_connection_t *c, uint8_t depth, uint8_t bpp)
{
  const xcb_setup_t *setup = xcb_get_setup(c);
  xcb_format_t *fmt = xcb_setup_pixmap_formats(setup);
  xcb_format_t *fmtend = fmt + xcb_setup_pixmap_formats_length(setup);

  for(; fmt != fmtend; ++fmt)
    {
	if((fmt->depth == depth) && (fmt->bits_per_pixel == bpp))
	  {
	    printf("fmt %p has pad %d depth %d, bpp %d\n", fmt,fmt->scanline_pad, fmt->depth, fmt->bits_per_pixel);
	    return fmt;
	  }
    }
  return 0;
}

/* ----------------------------------------------------------------------
--
-- main
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  int cairo = 0;

  if (argc > 1)
    {
      cairo = strtol(argv[1], NULL, 10);
    }
  if (cairo)
    printf("Enabling cairo\n");

  int screenNum;
  xcb_connection_t *c = xcb_connect(NULL, &screenNum);
  char *title = "Mandel test";

  const xcb_setup_t *setup = xcb_get_setup(c);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);  
  
  // we want the screen at index screenNum of the iterator
  for (int i = 0; i < screenNum; ++i)
    {
      xcb_screen_next(&iter);
    }

  unsigned int width = 1000;
  unsigned int height = 1000;
  unsigned int depth = 24;

  xcb_screen_t *s = iter.data;
  xcb_visualtype_t *visual = xcb_aux_find_visual_by_attrs(s, XCB_VISUAL_CLASS_TRUE_COLOR, depth);
  xcb_format_t *format;

  /* function produces a 8-bit images only */

  mandel_image *image = mandel_image_create_default(width, height, 0);
  mandel_image_create_hsv_palette(image);
  uint32_t stride;
  if (cairo)
    {
      stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, width);
    }
  else
    {
      format = find_format(c, 24, 32);
      stride = (format->bits_per_pixel >> 3) * width;
    }

  uint8_t* idata = calloc(sizeof(uint8_t), stride * height);
  for (int i = 0; i < height; i++)
    for (int j = 0; j < width; j++)
      {
	int v = image->data[i * width + j];
	idata[stride*i + j*4+0] = image->palette[v].red;
	idata[stride*i + j*4+1] = image->palette[v].green;
	idata[stride*i + j*4+2] = image->palette[v].blue;
      }

  cairo_surface_t *isurface, *wsurface;
  xcb_image_t *ximage;
  xcb_pixmap_t pmap;
  xcb_gc_t gc;

  if (cairo)
    {
      isurface = cairo_image_surface_create_for_data(idata, CAIRO_FORMAT_RGB24, width, height, stride);
    }
  else
    {
      ximage = xcb_image_create(width, height, XCB_IMAGE_FORMAT_Z_PIXMAP,
				format->scanline_pad,
				format->depth,
				format->bits_per_pixel,
				0,
				setup->image_byte_order,
				XCB_IMAGE_ORDER_LSB_FIRST,
				idata,
				stride * height,
				idata);
    }
  
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2];
  values[0] = s->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_BUTTON_PRESS;
  
  xcb_window_t w = xcb_generate_id (c);
  xcb_create_window (c, XCB_COPY_FROM_PARENT, w, s->root,
		     10, 10, width, height, 1,
		     XCB_WINDOW_CLASS_INPUT_OUTPUT,
		     visual->visual_id,
		     mask, values);
  
  /* set title on window */
  xcb_icccm_set_wm_name(c, w, XCB_ATOM_STRING, 8, strlen(title), title);
  
  /* set size hits on window */
  xcb_size_hints_t *hints = calloc(sizeof(xcb_size_hints_t), 1);
  xcb_icccm_size_hints_set_max_size(hints, width, height);
  xcb_icccm_size_hints_set_min_size(hints, width, height);
  xcb_icccm_set_wm_size_hints(c, w, XCB_ATOM_WM_NORMAL_HINTS, hints);

  if (cairo)
    {
      wsurface = cairo_xcb_surface_create(c, w,  visual, width, height);
    }
  else
    {
      /* create backing pixmap */
      pmap = xcb_generate_id(c);
      xcb_create_pixmap(c, 24, pmap, w, ximage->width, ximage->height);
      
      /* create pixmap plot gc */
      mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
      values[0] = s->black_pixel;
      values[1] = 0xffffff;
      
      gc = xcb_generate_id (c);
      xcb_create_gc (c, gc, pmap, mask, values);
  
      /* put the image into the pixmap */
      xcb_image_put(c, pmap, gc, ximage, 0, 0, 0);
    }

  /* show the window */
  printf("Mapping window\n");

  xcb_map_window (c, w);
  xcb_flush (c);
  
  int done = 0;
  xcb_generic_event_t *e;
  xcb_expose_event_t *ee;

  /* event loop */
  while (!done && (e = xcb_wait_for_event (c))) {
      switch (e->response_type) {
      case XCB_EXPOSE:
	ee=(xcb_expose_event_t *)e;
	printf ("expose %d,%d - %d,%d\n", ee->x,ee->y,ee->width,ee->height);
	if (cairo)
	  {
	    cairo_t *cr = cairo_create(wsurface);
	    cairo_set_source_surface(cr, isurface, 0, 0);
	    cairo_paint(cr);
	  }
	else
	  {
	    xcb_copy_area(c, pmap, w, gc,
			  ee->x,
			  ee->y,
			  ee->x,
			  ee->y,
			  ee->width,
			  ee->height);
	  }
	xcb_flush (c);
	//	image32+=16;
	break;
	
      case XCB_KEY_PRESS:
	/* exit on keypress */
	done = 1;
	break;
	
      case XCB_BUTTON_PRESS:
	//	fillimage(image->data, image->width, image->height);
	//      memset(image->data, 0, image32 - image->data);
	if (!cairo)
	  {
	    xcb_image_put(c, pmap, gc, ximage, 0, 0, 0);
	    xcb_copy_area(c, pmap, w, gc, 0,0,0,0,ximage->width,ximage->height);
	    xcb_flush (c);
	  }
	break;
      }
      free (e);
  }
  
  /* free pixmap */
  xcb_free_pixmap(c, pmap);
  
  /* close connection to server */
  xcb_disconnect (c);
  
  return 0;
}
