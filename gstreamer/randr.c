#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <xcb/xcb_aux.h>

int main()
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

  xcb_randr_crtc_t *crtcs = xcb_randr_get_screen_resources_crtcs(res);
  for (int c = 0; c < xcb_randr_get_screen_resources_crtcs_length(res); c++)
    {
      xcb_randr_get_crtc_info_reply_t *crtc = xcb_randr_get_crtc_info_reply(connection, xcb_randr_get_crtc_info(connection, crtcs[c], 0), 0);
      printf("Crtc %d: %dx%d+%d+%d\n", crtcs[c], crtc->width, crtc->height, crtc->x, crtc->y);
    }
}
