#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#include <xcb/xcb.h>
#include <xcb/xfixes.h>
#include <xcb/randr.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_renderutil.h>

#include <gst/gst.h>
#include <gst/video/videooverlay.h>

/* ----------------------------------------------------------------------
--
-- create_window
--
---------------------------------------------------------------------- */

#define MWM_HINTS_DECORATIONS (1L << 1)
#define PROP_MWM_HINTS_ELEMENTS 5
typedef struct {
  uint32_t flags;
  uint32_t functions;
  uint32_t decorations;
  int32_t input_mode;
  uint32_t status;
} MWMHints;

struct crtc_geometry {
  int x, y, width, height;
};

struct crtc_geometry** xcb_get_crts()
{
  int num_screen = 0;
  xcb_connection_t *connection = xcb_connect(0, &num_screen);
  
  uint32_t randr_major, randr_minor;
  
  xcb_randr_query_version_reply_t *rr_version;
  rr_version = xcb_randr_query_version_reply(connection, xcb_randr_query_version(connection, 1, 4), 0);
  
  randr_major = rr_version->major_version;
  randr_minor = rr_version->minor_version;
  
  printf("RANDR %d.%d\n", randr_major, randr_minor);

  xcb_screen_t *screen = xcb_aux_get_screen(connection, num_screen);

  xcb_randr_get_screen_info_reply_t *sc = xcb_randr_get_screen_info_reply(connection, xcb_randr_get_screen_info(connection, screen->root), 0);
  if (!sc) {
    fprintf(stderr, "Can't get ScreenInfo.\n");
    exit (1);
  }
  
  xcb_randr_get_screen_resources_reply_t *res = xcb_randr_get_screen_resources_reply(connection, xcb_randr_get_screen_resources(connection, screen->root), 0);
  
  printf("Number of outputs: %d\n", xcb_randr_get_screen_resources_outputs_length(res));
  xcb_randr_output_t *outputs = xcb_randr_get_screen_resources_outputs (res);
  for (int o = 0; o < xcb_randr_get_screen_resources_outputs_length(res); o++)
    {
      xcb_randr_get_output_info_reply_t * output = xcb_randr_get_output_info_reply (connection, xcb_randr_get_output_info(connection, outputs[o], 0), 0);

      int len = xcb_randr_get_output_info_name_length(output);
      char *name = alloca(len + 1);
      strncpy(name, xcb_randr_get_output_info_name(output), len);
      name[len] = '\0';

      int ncrts = xcb_randr_get_output_info_crtcs_length(output);
      printf("Output %d: %s - %d crts\n", outputs[o], name, ncrts);
    }

  int ncrtcs = xcb_randr_get_screen_resources_crtcs_length(res);

  struct crtc_geometry** geometry = calloc(sizeof(struct crtc_geometry*), ncrtcs+1);

  xcb_randr_crtc_t *crtcs = xcb_randr_get_screen_resources_crtcs(res);
  for (int c = 0; c < ncrtcs; c++)
    {
      xcb_randr_get_crtc_info_reply_t *crtc = xcb_randr_get_crtc_info_reply(connection, xcb_randr_get_crtc_info(connection, crtcs[c], 0), 0);
      printf("Crtc %d: %dx%d+%d+%d\n", crtcs[c], crtc->width, crtc->height, crtc->x, crtc->y);
      geometry[c] = calloc(sizeof(struct crtc_geometry), 1);
      struct crtc_geometry *g = geometry[c];
      g->x = crtc->x;
      g->y = crtc->y;
      g->width = crtc->width;
      g->height = crtc->height;
    }
  return geometry;
}

xcb_window_t create_window(xcb_connection_t *connection)
{
  if (connection == 0)
    {
      int num_screens;
      connection = xcb_connect(0, &num_screens);
    }

  char *title = "Foo";

  /* --------------------
     Use XCB to create window
     -------------------- */

  const xcb_setup_t *setup = xcb_get_setup(connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  xcb_screen_t *screen = iter.data;


  printf ("\n");
  printf ("Informations of screen %ld:\n", (long)screen->root);
  printf ("  bob->width.........: %d\n", screen->width_in_pixels);
  printf ("  bob->height........: %d\n", screen->height_in_pixels);
  printf ("  white pixel...: %ld\n", (long)screen->white_pixel);
  printf ("  black pixel...: %ld\n", (long)screen->black_pixel);
  printf ("  root depth....: %ld\n", (long)screen->root_depth);
  printf ("  visual id.....: %ld\n", (long)screen->root_visual);
  printf ("\n");

  /* --------------------
     Use 24-bit TrueColor visual
     -------------------- */


  uint32_t randr_major, randr_minor;

  xcb_randr_query_version_reply_t *rr_version;
  rr_version = xcb_randr_query_version_reply(connection, xcb_randr_query_version(connection, 1, 3), 0);

  randr_major = rr_version->major_version;
  randr_minor = rr_version->minor_version;

  printf("RANDR %d.%d\n", randr_major, randr_minor);


  xcb_visualtype_t *visual = xcb_aux_find_visual_by_attrs(screen, XCB_VISUAL_CLASS_TRUE_COLOR, 24);
  assert(visual);

  int depth = xcb_aux_get_depth_of_visual(screen, visual->visual_id);

  printf ("  window depth..: %ld\n", (long)depth);
  printf ("  visual id.....: %ld\n", (long)visual->visual_id);
  printf ("\n");

  /* --------------------
     Create new colormap
     -------------------- */

  xcb_colormap_t cmap = xcb_generate_id(connection);
  xcb_create_colormap(connection, XCB_COLORMAP_ALLOC_NONE, cmap, screen->root, visual->visual_id);
  xcb_aux_sync(connection);

  /* --------------------
     Create new window
     -------------------- */

  int width = screen->width_in_pixels;
  int height = screen->height_in_pixels;

  struct crtc_geometry **geometry = xcb_get_crts(connection);

  struct crtc_geometry *g = geometry[0];

  xcb_params_cw_t params;
  params.colormap = cmap;
  params.back_pixel = 0;
  params.border_pixel = 0;
  params.event_mask = XCB_EVENT_MASK_EXPOSURE |XCB_EVENT_MASK_STRUCTURE_NOTIFY;
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_COLORMAP | XCB_CW_EVENT_MASK;

  xcb_window_t window = xcb_generate_id(connection);
  xcb_aux_create_window_checked(connection,            /* Connection          */
				 depth,                            /* depth (same as root)*/
				window,                        /* window Id           */
				screen->root,                  /* parent window       */
				g->x,
				g->y,                          /* x, y                */
				g->width,
				g->height,	       	       /* width, height       */
				10,                            /* border_width        */
				XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
				visual->visual_id,             /* visual              */
				mask, &params);                /* masks and params    */



  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);
  MWMHints mwmhints;

  xcb_intern_atom_reply_t* XA_NO_BORDER = xcb_intern_atom_reply(connection, xcb_intern_atom(connection, 0, strlen("_MOTIF_WM_HINTS"), "_MOTIF_WM_HINTS"), 0);

  mwmhints.flags = MWM_HINTS_DECORATIONS;
  mwmhints.decorations = 0;

  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XA_NO_BORDER->atom, XCB_ATOM, 32, PROP_MWM_HINTS_ELEMENTS, &mwmhints);

  xcb_map_window(connection, window);
  xcb_flush(connection);
  xcb_xfixes_hide_cursor(connection, window);
  return window;
}

GstBusSyncReply create_window_gst(GstBus *bus, GstMessage *message, xcb_connection_t *connection)
{
  if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
    return GST_BUS_PASS;
  
  if (!gst_structure_has_name (gst_message_get_structure(message), "prepare-window-handle"))
    return GST_BUS_PASS;

  xcb_window_t win = create_window(connection);
  gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY(GST_MESSAGE_SRC (message)), win);
  
  gst_message_unref (message);
  return GST_BUS_DROP;
}
