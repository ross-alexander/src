/* ----------------------------------------------------------------------

   wayland client example using xdg_toplevel & wl_shm

   ---------------------------------------------------------------------- */

#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

#include <cairo.h>
#include <glycin.h>
#include <babl/babl.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

struct tile_surface_t {
  int width;
  int height;
  cairo_surface_t *surface;
  cairo_t *cairo;
};


/* Shared memory support code */
static void randname(char *buf)
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  long r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i)
    {
      buf[i] = 'A'+(r&15)+(r&16)*2;
      r >>= 5;
    }
}

static int create_shm_file(void)
{
  int retries = 100;
  do {
    char name[] = "/wl_shm-XXXXXX";
    randname(name + sizeof(name) - 7);
    --retries;
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0)
      {
	shm_unlink(name);
	return fd;
      }
  } while (retries > 0 && errno == EEXIST);
  return -1;
}

static int allocate_shm_file(size_t size)
{
  int fd = create_shm_file();
  if (fd < 0)
    return -1;
  int ret;
  do {
    ret = ftruncate(fd, size);
  } while (ret < 0 && errno == EINTR);
  if (ret < 0)
    {
      close(fd);
      return -1;
    }
  return fd;
}


enum pointer_event_mask {
  POINTER_EVENT_ENTER = 1 << 0,
  POINTER_EVENT_LEAVE = 1 << 1,
  POINTER_EVENT_MOTION = 1 << 2,
  POINTER_EVENT_BUTTON = 1 << 3,
  POINTER_EVENT_AXIS = 1 << 4,
  POINTER_EVENT_AXIS_SOURCE = 1 << 5,
  POINTER_EVENT_AXIS_STOP = 1 << 6,
  POINTER_EVENT_AXIS_DISCRETE = 1 << 7,
};

struct pointer_event {
  uint32_t event_mask;
  wl_fixed_t surface_x, surface_y;
  uint32_t button, state;
  uint32_t time;
  uint32_t serial;
  struct {
    bool valid;
    wl_fixed_t value;
    int32_t discrete;
  } axes[2];
  uint32_t axis_source;
};


/* Wayland code */
struct client_state {
    /* Globals */
  struct wl_display *wl_display;
  struct wl_registry *wl_registry;
  struct wl_shm *wl_shm;
  struct wl_compositor *wl_compositor;
  struct xdg_wm_base *xdg_wm_base;
  /* Objects */
  struct wl_surface *wl_surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct wl_seat *wl_seat;
  struct wl_keybord *wl_keyboard;
  struct wl_pointer *wl_pointer;
  struct wl_touch *wl_touch;
  struct pointer_event pointer_event;
  /* toplevel */
  int width, height;
  bool closed;
  /* Lua */
  lua_State *luastate;
};

static void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
			     uint32_t serial, struct wl_surface *surface,
			     wl_fixed_t surface_x, wl_fixed_t surface_y)
{
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_ENTER;
  client_state->pointer_event.serial = serial;
  client_state->pointer_event.surface_x = surface_x;
  client_state->pointer_event.surface_y = surface_y;
}

static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
			     uint32_t serial, struct wl_surface *surface)
{
  struct client_state *client_state = data;
  client_state->pointer_event.serial = serial;
  client_state->pointer_event.event_mask |= POINTER_EVENT_LEAVE;
}

static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time,
			       wl_fixed_t surface_x, wl_fixed_t surface_y)
{
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_MOTION;
  client_state->pointer_event.time = time;
  client_state->pointer_event.surface_x = surface_x, client_state->pointer_event.surface_y = surface_y;
}

static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial,
			      uint32_t time, uint32_t button, uint32_t state)
{
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_BUTTON;
  client_state->pointer_event.time = time;
  client_state->pointer_event.serial = serial;
  client_state->pointer_event.button = button, client_state->pointer_event.state = state;
}

static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time,
			    uint32_t axis, wl_fixed_t value)
{
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS;
  client_state->pointer_event.time = time;
  client_state->pointer_event.axes[axis].valid = true;
  client_state->pointer_event.axes[axis].value = value;
}

static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source)
{
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_SOURCE;
  client_state->pointer_event.axis_source = axis_source;
}

static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis)
{
  struct client_state *client_state = data;
  client_state->pointer_event.time = time;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_STOP;
  client_state->pointer_event.axes[axis].valid = true;
}

static void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
				     uint32_t axis, int32_t discrete)
{
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_DISCRETE;
  client_state->pointer_event.axes[axis].valid = true;
  client_state->pointer_event.axes[axis].discrete = discrete;
}

static void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer)
{
  struct client_state *client_state = data;
  struct pointer_event *event = &client_state->pointer_event;
  //   fprintf(stderr, "pointer frame @ %d: ", event->time);

  if (event->event_mask & POINTER_EVENT_ENTER)
    {
      // fprintf(stderr, "entered %f, %f ", wl_fixed_to_double(event->surface_x), wl_fixed_to_double(event->surface_y));
    }
  
  if (event->event_mask & POINTER_EVENT_LEAVE)
    {
      // fprintf(stderr, "leave");
    }

  if (event->event_mask & POINTER_EVENT_MOTION)
    {
      // fprintf(stderr, "motion %f, %f ", wl_fixed_to_double(event->surface_x), wl_fixed_to_double(event->surface_y));
    }

  if (event->event_mask & POINTER_EVENT_BUTTON)
    {
      char *state = event->state == WL_POINTER_BUTTON_STATE_RELEASED ? "released" : "pressed";
      fprintf(stderr, "button %d %s ", event->button, state);
      if (event->state != WL_POINTER_BUTTON_STATE_RELEASED)
	{
	  if (event->button == 272)
	    xdg_toplevel_move(client_state->xdg_toplevel, client_state->wl_seat, event->serial);
	  if (event->button == 273)
	    xdg_toplevel_resize(client_state->xdg_toplevel, client_state->wl_seat, event->serial, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT);
	}
    }
  uint32_t axis_events = POINTER_EVENT_AXIS
    | POINTER_EVENT_AXIS_SOURCE
    | POINTER_EVENT_AXIS_STOP
    | POINTER_EVENT_AXIS_DISCRETE;
  char *axis_name[2] = {
    [WL_POINTER_AXIS_VERTICAL_SCROLL] = "vertical",
    [WL_POINTER_AXIS_HORIZONTAL_SCROLL] = "horizontal",
  };
  char *axis_source[4] = {
    [WL_POINTER_AXIS_SOURCE_WHEEL] = "wheel",
    [WL_POINTER_AXIS_SOURCE_FINGER] = "finger",
    [WL_POINTER_AXIS_SOURCE_CONTINUOUS] = "continuous",
    [WL_POINTER_AXIS_SOURCE_WHEEL_TILT] = "wheel tilt",
  };
  if (event->event_mask & axis_events) {
    for (size_t i = 0; i < 2; ++i) {
      if (!event->axes[i].valid) {
	continue;
      }
      fprintf(stderr, "%s axis ", axis_name[i]);
      if (event->event_mask & POINTER_EVENT_AXIS) {
	fprintf(stderr, "value %f ", wl_fixed_to_double(event->axes[i].value));
      }
      if (event->event_mask & POINTER_EVENT_AXIS_DISCRETE) {
	fprintf(stderr, "discrete %d ",
		event->axes[i].discrete);
      }
      if (event->event_mask & POINTER_EVENT_AXIS_SOURCE) {
	fprintf(stderr, "via %s ", axis_source[event->axis_source]);
      }
      if (event->event_mask & POINTER_EVENT_AXIS_STOP) {
	fprintf(stderr, "(stopped) ");
      }
    }
  }
  
  // fprintf(stderr, "\n");
  memset(event, 0, sizeof(*event));
}

static const struct wl_pointer_listener wl_pointer_listener = {
  .enter = wl_pointer_enter,
  .leave = wl_pointer_leave,
  .motion = wl_pointer_motion,
  .button = wl_pointer_button,
  .axis = wl_pointer_axis,
  .frame = wl_pointer_frame,
  .axis_source = wl_pointer_axis_source,
  .axis_stop = wl_pointer_axis_stop,
  .axis_discrete = wl_pointer_axis_discrete,
};

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    /* Sent by the compositor when it's no longer using this buffer */
  wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
  .release = wl_buffer_release,
};

/* ----------------------------------------------------------------------

   draw_frame

   ---------------------------------------------------------------------- */

static struct wl_buffer *draw_frame(struct client_state *state)
{
  const int width = state->width, height = state->height;
  int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
  int size = stride * height;

  /* Create shared memory file */
  
  int fd = memfd_create("buffer", 0);
  if (fd == -1)
    {
      return NULL;
    }
  ftruncate(fd, size);

  /* Map file into virtual memory */
  
  uint32_t *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED)
    {
      close(fd);
      return NULL;
    }
  
  /* Create wayland shared memory buffer */

  struct wl_shm_pool *pool = wl_shm_create_pool(state->wl_shm, fd, size);
  struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);
  wl_shm_pool_destroy(pool);
  close(fd);

  fprintf(stdout, "draw_frame: %d × %d × %d\n", width, height, stride);

  /* Create cairo surface on memory mapped region */
  
  cairo_surface_t *surface = cairo_image_surface_create_for_data((void*)data, CAIRO_FORMAT_ARGB32, width, height, stride);
  cairo_t *cairo = cairo_create(surface);

  int draw_type = lua_getglobal(state->luastate, "draw");
  if (draw_type != LUA_TNIL)
    {
      if (draw_type == LUA_TFUNCTION)
	{
	  struct tile_surface_t *tile_surface = calloc(sizeof(struct tile_surface_t), 1);
	  tile_surface->width = width;
	  tile_surface->height = height;
	  tile_surface->surface = surface;
	  tile_surface->cairo = cairo;
	  struct tile_surface_t **handle = lua_newuserdata(state->luastate, sizeof(struct tile_surface_t*));
	  *handle = tile_surface;
	  luaL_setmetatable(state->luastate, "tile_surface_t");
	  lua_pcall(state->luastate, 1, 0, 0);
	}
      else
	{
	  lua_pop(state->luastate, 1);
	}
    }
  else
    {
      cairo_set_source_rgb(cairo, 1.0, 0.0, 0.0);
      cairo_pattern_t *p = cairo_pattern_create_linear(0, 0, width, height);
  
      /* offset, red, green, blue, alpha */
      cairo_pattern_add_color_stop_rgba(p, 0, 1, 0, 0, 1.0);
      cairo_pattern_add_color_stop_rgba(p, 1, 0, 1, 0, 1.0);
      
      cairo_set_source(cairo, p);
      cairo_rectangle(cairo, 0, 0, width, height);
      cairo_fill(cairo);
    }
  cairo_destroy(cairo);
  cairo_surface_destroy(surface);
  
  munmap(data, size);
  wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);
  return buffer;
}

/* ----------------------------------------------------------------------
   --
   -- xdg_surface_configure
   --
   ---------------------------------------------------------------------- */

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
  struct client_state *state = data;
  xdg_surface_ack_configure(xdg_surface, serial);

  struct wl_buffer *buffer = draw_frame(state);
  wl_surface_attach(state->wl_surface, buffer, 0, 0);
  wl_surface_commit(state->wl_surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
  .configure = xdg_surface_configure,
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
  xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

/* ----------------------------------------------------------------------
   --
   -- wl_seat
   --
   ---------------------------------------------------------------------- */

static void wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities)
{
  struct client_state *state = data;
  
  bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;
  
  if (have_pointer && state->wl_pointer == NULL)
    {
      state->wl_pointer = wl_seat_get_pointer(state->wl_seat);
      wl_pointer_add_listener(state->wl_pointer, &wl_pointer_listener, state);
    }
  else if (!have_pointer && state->wl_pointer != NULL)
    {
      wl_pointer_release(state->wl_pointer);
      state->wl_pointer = NULL;
    }
}

static void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name)
{
  fprintf(stderr, "seat name: %s\n", name);
}

static const struct wl_seat_listener wl_seat_listener = {
  .capabilities = wl_seat_capabilities,
  .name = wl_seat_name,
};


/* ----------------------------------------------------------------------
   --
   -- registry_global
   --
   ---------------------------------------------------------------------- */

static void registry_global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version)
{
    struct client_state *state = data;
    if (strcmp(interface, wl_shm_interface.name) == 0)
      {
        state->wl_shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, 1);
      }
    else if (strcmp(interface, wl_compositor_interface.name) == 0)
      {
        state->wl_compositor = wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
      }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
      {
        state->xdg_wm_base = wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->xdg_wm_base,&xdg_wm_base_listener, state);
      }
    else if (strcmp(interface, wl_seat_interface.name) == 0)
      {
	state->wl_seat = wl_registry_bind(wl_registry, name, &wl_seat_interface, 7);
	wl_seat_add_listener(state->wl_seat, &wl_seat_listener, state);
      }
}


static void registry_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name)
{
    /* This space deliberately left blank */
}

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

/* ----------------------------------------------------------------------
   --
   -- xdg_toplevel_surface
   --
   ---------------------------------------------------------------------- */

static void xdg_toplevel_configure(void *data,
				   struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height,
				   struct wl_array *states)
{
  struct client_state *state = data;
  if (width == 0 || height == 0) {
    /* Compositor is deferring to us */
    return;
  }
  state->width = width;
  state->height = height;
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel)
{
  struct client_state *state = data;
  state->closed = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
  .configure = xdg_toplevel_configure,
  .close = xdg_toplevel_close,
};


/* ----------------------------------------------------------------------

   tile_surface_init

   ---------------------------------------------------------------------- */

int tile_surface_t___tostring(lua_State *L)
{
  lua_pushstring(L, "tile_surface_t");
  return 1;
}

int tile_surface_t_new(lua_State *L)
{
  struct tile_surface_t *surface = calloc(sizeof(struct tile_surface_t), 1);
  struct tile_surface_t **handle = (struct tile_surface_t**)lua_newuserdata(L, sizeof(struct tile_surface_t*));
  luaL_setmetatable(L, "tile_surface_t");
  *handle = surface;
  return 1;
}

int tile_surface_t_set_source_rgb(lua_State *L)
{
  struct tile_surface_t** handle = (struct tile_surface_t**)luaL_checkudata(L, 1, "tile_surface_t");
  double r = lua_tonumber(L, 2);
  double g = lua_tonumber(L, 3);
  double b = lua_tonumber(L, 4);
  printf("set_source_rgb(%4.2f %4.2f %4.2f)\n", r, g, b);
  cairo_set_source_rgb((*handle)->cairo, r, g, b);
  return 0;
}

int tile_surface_t_rectangle(lua_State *L)
{
  struct tile_surface_t** handle = (struct tile_surface_t**)luaL_checkudata(L, 1, "tile_surface_t");
  double x = lua_tonumber(L, 2);
  double y = lua_tonumber(L, 3);
  double w = lua_tonumber(L, 4);
  double h = lua_tonumber(L, 5);
  printf("set_rectangle(%4.2f %4.2f %4.2f %4.2f)\n", x, y, w, h);
  cairo_rectangle((*handle)->cairo, x, y, w, h);
  return 0;
}

int tile_surface_t_fill(lua_State *L)
{
  struct tile_surface_t** handle = (struct tile_surface_t**)luaL_checkudata(L, 1, "tile_surface_t");
  printf("fill()\n");
  cairo_fill((*handle)->cairo);
  return 0;
}

int tile_surface_t_configuration(lua_State *L)
{
  struct tile_surface_t* tile = *(struct tile_surface_t**)luaL_checkudata(L, 1, "tile_surface_t");
  lua_newtable(L);
  lua_pushinteger(L, tile->width);
  lua_setfield(L, -2, "width");
  lua_pushinteger(L, tile->height);
  lua_setfield(L, -2, "height");
  return 1;
}

/* ----------------------------------------------------------------------
   --
   -- tile_surface_set_source_file
   --
   ---------------------------------------------------------------------- */

int tile_surface_t_set_source_file(lua_State *L)
{
  GError *error;
  struct tile_surface_t** handle = (struct tile_surface_t**)luaL_checkudata(L, 1, "tile_surface_t");
  struct tile_surface_t* tile = *handle;
  
  const char *path = luaL_checkstring(L, 2);

  printf("tile_surface_t_set_source_file(%s)\n", path);

  GFile *file = g_file_new_for_path(path);
  GlyLoader *loader = gly_loader_new(file);

  /* --------------------
     -- Load image and return 0 if fails
     -------------------- */
  
  GlyImage *image = gly_loader_load(loader, &error);
  if (image == nullptr)
    {
      fprintf(stderr, "Failed to load %s: %s\n", path, error->message);
      return 0;
    }
  
  printf("Image %s loaded\n", path);

  /* --------------------
     Get first and probably only frame, exit if fails
     -------------------- */
  
  GlyFrame *frame = gly_image_next_frame(image, &error);

  if (frame == nullptr)
    {
      fprintf(stderr, "Failed to load frame: %s\n", error->message);
      g_object_unref(image);
      g_object_unref(loader);
      return 0;
    }

  /* --------------------
     Get format and create Babl
     -------------------- */
  
  GlyMemoryFormat format = gly_frame_get_memory_format(frame);
  char *description = nullptr;
  const Babl *babl_format_src = nullptr;

  switch(format)
    {
    case GLY_MEMORY_R8G8B8:
      babl_format_src = babl_format_with_space("R'G'B' u8", NULL);
      description = "8-bit RGB";
      break;
    default:
      babl_format_src = nullptr;
      description = "Unsupported format";
      break;
    }
  uint32_t height = gly_frame_get_height(frame);
  uint32_t width = gly_frame_get_width(frame);
  uint32_t stride_src = gly_frame_get_stride(frame);
  gboolean alpha = gly_memory_format_has_alpha(format);

  if (babl_format_src)
    {
      GBytes *src = gly_frame_get_buf_bytes(frame);
      gsize buffer_src_size;
      gconstpointer buffer_src = g_bytes_get_data(src, &buffer_src_size);
      
      printf("Size should be %d, has %ld\n", height * stride_src, buffer_src_size);
      
      cairo_format_t cairo_format = alpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24;

      uint32_t stride_dst = cairo_format_stride_for_width(cairo_format, width);

      void *buffer_dst = malloc(stride_dst * height);
      
      cairo_surface_t* surface = cairo_image_surface_create_for_data(buffer_dst, cairo_format, width, height, stride_dst);
      const Babl *babl_format_dst = alpha ? babl_format("cairo-ARGB32") : babl_format("cairo-RGB24");

      printf("Converting from %s [%s] format %s [%d] to %s [%d]\n", description, alpha ? "alpha" : "no alpha",
	     babl_get_name(babl_format_src),
	     babl_format_get_bytes_per_pixel(babl_format_src),
	     babl_get_name(babl_format_dst),
	     babl_format_get_bytes_per_pixel(babl_format_dst)
	     );

      printf("Image width [%d] height [%d]\n", width, height);
      
      printf("Source stride [%d] Destination stride [%d]\n", stride_src, stride_dst);
      
      babl_process_rows(babl_fish(babl_format_src, babl_format_dst),
			buffer_src,
			stride_src,
			buffer_dst,
			stride_dst,
			width,
			height);

      if (tile->cairo)
	cairo_set_source_surface(tile->cairo, surface, 0.0, 0.0);

      if (!tile->surface)
	{
	  tile->surface = surface;
	  tile->width = width;
	  tile->height = height;
	  printf("Setting surface to %d × %d\n", width, height);
	}
    }
  g_object_unref(frame);
  g_object_unref(image);
  g_object_unref(loader);
  
  return 0;
}

int tile_surface_t_set_source_tile(lua_State *L)
{
  struct tile_surface_t* tile = *(struct tile_surface_t**)luaL_checkudata(L, 1, "tile_surface_t");
  struct tile_surface_t* src = *(struct tile_surface_t**)luaL_checkudata(L, 2, "tile_surface_t");

  assert(tile->cairo);
  assert(src->surface);
  
  if (tile->cairo && src->surface)
    {
      cairo_set_source_surface(tile->cairo, src->surface, 0.0, 0.0);
      printf("Setting source to surface [%d × %d]\n", src->width, src->height);
    }
  return 0;
}
  
/* ----------------------------------------------------------------------
   --
   -- lua init
   --
   ---------------------------------------------------------------------- */

void tile_surface_init(lua_State *L)
{
  const luaL_Reg tile_surface_t_instance_methods[] = {
    {"set_source_file",		tile_surface_t_set_source_file},
    {"set_source_rgb",		tile_surface_t_set_source_rgb},
    {"set_source_tile",		tile_surface_t_set_source_tile},
    {"rectangle",		tile_surface_t_rectangle},
    {"fill",			tile_surface_t_fill},
    {"configuration",		tile_surface_t_configuration},
    {0, 0}
  };

  const luaL_Reg tile_surface_t_class_methods[] = {
    {"new",			tile_surface_t_new},
    { 0, 0}
  };

  const luaL_Reg tile_surface_t_meta_methods[] = {
    {"__tostring", tile_surface_t___tostring},
    {0, 0},
  };

  luaL_newmetatable(L, "tile_surface_t");
  luaL_setfuncs(L, tile_surface_t_meta_methods, 0);

  /* instance methods */

  lua_newtable(L);
  luaL_setfuncs(L, tile_surface_t_instance_methods, 0);
  lua_setfield(L, -2, "__index");

  //  lua_pushliteral(L, "__index");
  //  lua_rawset(L, -2);                  /* metatable.__index = methods */

  lua_pop(L, 1); // metatable
  
  lua_pushglobaltable(L);
  lua_newtable(L);
  luaL_setfuncs(L, tile_surface_t_class_methods, 0);
  //  lua_pushcclosure(L, tile_surface_t_new, 0); lua_setfield(L, -2, "new");
  lua_setfield(L, -2, "tile_surface_t");
}

/* ----------------------------------------------------------------------

   main

   ---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  struct client_state state = { 0 };
  babl_init();

  /* lua */
  state.luastate = luaL_newstate();
  tile_surface_init(state.luastate);
  luaL_openselectedlibs(state.luastate, LUA_GLIBK|LUA_IOLIBK, 0);
  int ret = luaL_dofile(state.luastate, "client.lua");
  if (ret != 0)
    {
      fprintf(stderr, "%s\n", lua_tostring(state.luastate, -1));
      exit(1);
    }
  
  /* wayland display */

  state.width = 640;
  state.height = 480;
  
  state.wl_display = wl_display_connect(NULL);
  assert(state.wl_display);

  /* get registry */
  
  state.wl_registry = wl_display_get_registry(state.wl_display);
  wl_registry_add_listener(state.wl_registry, &wl_registry_listener, &state);
  wl_display_roundtrip(state.wl_display);

  /* Create surface with compositor */
  
  state.wl_surface = wl_compositor_create_surface(state.wl_compositor);

  /* Use XDG surface */
  
  state.xdg_surface = xdg_wm_base_get_xdg_surface(state.xdg_wm_base, state.wl_surface);
  xdg_surface_add_listener(state.xdg_surface, &xdg_surface_listener, &state);

  /* Get toplevel surface */
  
  state.xdg_toplevel = xdg_surface_get_toplevel(state.xdg_surface);
  xdg_toplevel_add_listener(state.xdg_toplevel, &xdg_toplevel_listener, &state);
  xdg_toplevel_set_title(state.xdg_toplevel, "Example client");

  wl_surface_commit(state.wl_surface);

  while (wl_display_dispatch(state.wl_display))
    {
      /* This space deliberately left blank */
    }
    return 0;
}
