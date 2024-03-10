/* ----------------------------------------------------------------------
   --
   -- surfers
   --
   -- 2024-03-07: Wayland/cairo example

   -- https://wayland.app/protocols/wayland
   --
   ---------------------------------------------------------------------- */



#include <stdio.h>
#include <string.h>

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

struct surfers_t {
  /* globals */
  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *wl_compositor; // https://wayland.freedesktop.org/docs/html/apa.html#protocol-spec-wl_compositor 
  //  struct xdg_wm_base *xdg_wm_base;

  /* objects */
 struct wl_surface *wl_surface; // https://wayland.freedesktop.org/docs/html/apa.html#protocol-spec-wl_surface
  
};

/* ----------------------------------------------------------------------
   --
   -- registry_global
   --
   ---------------------------------------------------------------------- */

static void registry_global(void *data, struct wl_registry *wl_registry, uint32_t index, const char *interface, uint32_t version)
{
  surfers_t *surfers = (surfers_t*)data;
  printf("registry_global -- [%d] %s [%d]\n", index, interface, version);
  if (strcmp(interface, wl_shm_interface.name) == 0)
    {
    //     surfers->wl_shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, 1);
    }
  else if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
      //      surfers->wl_compositor = (wl_compositor*)wl_registry_bind(wl_registry, index, &wl_compositor_interface, 4);
    }
  else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
      // surfers->xdg_wm_base = (xdg_wm_base*)wl_registry_bind(wl_registry, index, &xdg_wm_base_interface, 1);
    //     xdg_wm_base_add_listener(state->xdg_wm_base, &xdg_wm_base_listener, state);
    }
    // else if (strcmp(interface, wl_seat_interface.name) == 0)
    //   {
    // 	state->wl_seat = wl_registry_bind(wl_registry, name, &wl_seat_interface, 7);
    // 	wl_seat_add_listener(state->wl_seat, &wl_seat_listener, state);
    //   }
}


static void registry_global_remove(void *data, struct wl_registry *wl_registry, uint32_t index)
{
  printf("registry_global_remove\n");
    /* This space deliberately left blank */
}

/* ----------------------------------------------------------------------
   --
   -- main
   --
   ---------------------------------------------------------------------- */

int main(int argc, char **argv)
{
  surfers_t *surfers = new surfers_t;

  // Connect to display
  
  surfers->display = wl_display_connect(NULL);
    if (surfers->display == nullptr)
      {
	fprintf(stderr, "Can't connect to display\n");
	exit(1);
      }
    printf("connected to display\n");


    /* --------------------
    Get registry
    -------------------- */

    const struct wl_registry_listener wl_registry_listener = {
      .global = registry_global,
      .global_remove = registry_global_remove,
    };
    
    surfers->registry = wl_display_get_registry(surfers->display);
    wl_registry_add_listener(surfers->registry, &wl_registry_listener, NULL);
    wl_display_dispatch(surfers->display);
    wl_display_roundtrip(surfers->display);
}
