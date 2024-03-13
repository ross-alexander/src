/* ----------------------------------------------------------------------
   --
   -- surfers
   --
   -- 2024-03-07: Wayland/cairo example

   -- https://wayland.app/protocols/wayland
   -- https://wayland.freedesktop.org/docs/html/apa.html#protocol-spec-wl_compositor 
   -- https://wayland.freedesktop.org/docs/html/apa.html#protocol-spec-wl_surface
   --
   ---------------------------------------------------------------------- */

#include <cassert>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include <cairo.h>

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

struct surfers_t {
  /* globals */
  
  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *wl_compositor;
  struct wl_shm *wl_shm;
  struct xdg_wm_base *xdg_wm_base;
  
  /* objects */
  struct wl_surface *wl_surface;
  struct wl_buffer *wl_buffer;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
};



/* ----------------------------------------------------------------------
   --
   -- xdg_surface_configure
   --
   ---------------------------------------------------------------------- */

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
  printf("wl_buffer_release\n");
  surfers_t *surfers = (surfers_t*)data;
    /* Sent by the compositor when it's no longer using this buffer */
  wl_buffer_destroy(surfers->wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
  .release = wl_buffer_release,
};


static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
  surfers_t *surfers = (surfers_t*)data;
  xdg_surface_ack_configure(xdg_surface, serial);

  int width = 600;
  int height = 400;
  int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
  int size = height * stride;

  printf("xdg_surface_configure [%d × %d × %d]\n", width, height, stride);

  //  https://bugaevc.gitbooks.io/writing-wayland-clients/content/black-square/allocating-a-buffer.html

  int fd = memfd_create("buffer", 0);
  printf("Create memfd %d\n", fd);
  ftruncate(fd, size);

  unsigned char *u_buffer = (unsigned char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  struct wl_shm_pool *pool = wl_shm_create_pool(surfers->wl_shm, fd, size);

  printf("Create shm_pool\n");

  surfers->wl_buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);

  printf("Create buffer\n");
  
  wl_shm_pool_destroy(pool);

  printf("Destroy pool\n");
  close(fd);

  printf("Close fd %d\n", fd);

  /* --------------------
     Use cairo with ARGB32
     -------------------- */
  
  cairo_surface_t *surface = cairo_image_surface_create_for_data((unsigned char*)u_buffer, CAIRO_FORMAT_ARGB32, width, height, stride);
  cairo_t *cairo = cairo_create(surface);
  cairo_set_source_rgb(cairo, 1.0, 0.0, 0.0);
  
  cairo_pattern_t *p = cairo_pattern_create_linear(0, 0, width, height);
  
  /* offset, red, green, blue, alpha */
  cairo_pattern_add_color_stop_rgba(p, 0, 1, 0, 0, 1.0);
  cairo_pattern_add_color_stop_rgba(p, 1, 0, 1, 0, 1.0);
  
  cairo_set_source(cairo, p);
  cairo_rectangle(cairo, 0, 0, width, height);
  cairo_fill(cairo);
  cairo_destroy(cairo);
  cairo_surface_destroy(surface);

  /* --------------------
     Unmap shared memory buffer
     -------------------- */
  
  munmap(u_buffer, size);

  wl_buffer_add_listener(surfers->wl_buffer, &wl_buffer_listener, surfers);

  printf("Add release listener\n");

  wl_surface_attach(surfers->wl_surface, surfers->wl_buffer, 0, 0);

  printf("Attach buffer to surface\n");
  
  wl_surface_commit(surfers->wl_surface);
  printf("Commit surface\n\n");
}

/* ----------------------------------------------------------------------
   --
   -- xdg_wm_base
   --
   ---------------------------------------------------------------------- */

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
  //  surfers_t* surfers = (surfers_t*)data;
  printf("Ping -- Pong\n");
  xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
  .ping = xdg_wm_base_ping,
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
      surfers->wl_shm = (wl_shm*)wl_registry_bind(wl_registry, index, &wl_shm_interface, 1);
    }
  else if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
      surfers->wl_compositor = (wl_compositor*)wl_registry_bind(wl_registry, index, &wl_compositor_interface, 4);
    }
  else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
      surfers->xdg_wm_base = (xdg_wm_base*)wl_registry_bind(wl_registry, index, &xdg_wm_base_interface, 1);

      // Add ping-pong for compositor to determine if a client is still alive
      
      xdg_wm_base_add_listener(surfers->xdg_wm_base, &xdg_wm_base_listener, surfers);
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

static const struct wl_registry_listener wl_registry_listener = {
      .global = registry_global,
      .global_remove = registry_global_remove,
    };
    

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

    surfers->registry = wl_display_get_registry(surfers->display);
    wl_registry_add_listener(surfers->registry, &wl_registry_listener, surfers);
    wl_display_roundtrip(surfers->display);

    /* --------------------
       Create surface and then make it an xdg_surface
       -------------------- */
    
  surfers->wl_surface = wl_compositor_create_surface(surfers->wl_compositor);
  surfers->xdg_surface = xdg_wm_base_get_xdg_surface(surfers->xdg_wm_base, surfers->wl_surface);

  const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
  };
  xdg_surface_add_listener(surfers->xdg_surface, &xdg_surface_listener, surfers);

  /* --------------------
     Make the surface a top_level surface
     https://wayland.app/protocols/xdg-shell#xdg_toplevel
     -------------------- */
  
  surfers->xdg_toplevel = xdg_surface_get_toplevel(surfers->xdg_surface);
  xdg_toplevel_set_title(surfers->xdg_toplevel, "Example client");

  /* Commit pending state to current state */
  
  wl_surface_commit(surfers->wl_surface);

  printf("Enter dispatch loop\n");

  while (wl_display_dispatch(surfers->display))
    {
      /* This space deliberately left blank */
    }
    return 0;

}
