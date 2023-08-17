/* ----------------------------------------------------------------------
   --
   -- draw
   --
   -- 2023-08-09: Add cairo image surface
   --
   -- 2023-08-07: Fix GBM issue with gbm_bo_map & gbm_bo_unmap
   --   Add GEGL
   -- 2023-08-03: Draw directly into a DRM framebuffer
   --
   ---------------------------------------------------------------------- */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>

#include <getopt.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <drm_fourcc.h>
#include <gbm.h>
#include <gegl.h>
#include <cairo.h>

/* ----------------------------------------------------------------------
   --
   -- structs
   --
   ---------------------------------------------------------------------- */

typedef struct modeset_dev {
  struct modeset_dev *next;
  uint32_t conn;
  uint32_t crtc;
  drmModeModeInfo mode;

  uint32_t width;
  uint32_t height;
  uint32_t stride;
  uint32_t size;
  uint32_t handle;
  uint8_t *map;
  uint32_t fb;

  struct gbm_bo* bo;
  
  drmModeCrtc *saved_crtc;
} modeset_dev;

/* ----------------------------------------------------------------------
   --
   -- modset_create_fb
   --
   ---------------------------------------------------------------------- */

int modeset_create_fb(int fd, modeset_dev *dev)
{
  struct drm_mode_create_dumb creq;
  struct drm_mode_destroy_dumb dreq;
  struct drm_mode_map_dumb mreq;
  int ret;
  
  /* create dumb buffer */

  memset(&creq, 0, sizeof(creq));
  creq.width = dev->width;
  creq.height = dev->height;
  creq.bpp = 32;
  ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
  if (ret < 0)
    {
      fprintf(stderr, "cannot create dumb buffer (%d): %m\n", errno);
      return -errno;
    }

  dev->stride = creq.pitch;
  dev->size = creq.size;
  dev->handle = creq.handle;

  /* create framebuffer object for the dumb-buffer */

  ret = drmModeAddFB(fd, dev->width, dev->height, 24, 32, dev->stride, dev->handle, &dev->fb);
  if (ret)
    {
      fprintf(stderr, "cannot create framebuffer (%d): %m\n", errno);
      ret = -errno;
      goto err_destroy;
    }
  
  /* prepare buffer for memory mapping */
  
  memset(&mreq, 0, sizeof(mreq));
  mreq.handle = dev->handle;
  ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
  if (ret)
    {
      fprintf(stderr, "cannot map dumb buffer (%d): %m\n", errno);
      ret = -errno;
      goto err_fb;
    }
  
  /* perform actual memory mapping */

  dev->map = mmap(0, dev->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);
  if (dev->map == MAP_FAILED)
    {
      fprintf(stderr, "cannot mmap dumb buffer (%d): %m\n", errno);
      ret = -errno;
      goto err_fb;
    }
  
  /* clear the framebuffer to 0 */
  memset(dev->map, 0, dev->size);
  return 0;
  
 err_fb:
  drmModeRmFB(fd, dev->fb);
 err_destroy:
  memset(&dreq, 0, sizeof(dreq));
  dreq.handle = dev->handle;
  drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
  return ret;
}

/* ----------------------------------------------------------------------
   --
   -- modeset_find_crtc
   --
   ---------------------------------------------------------------------- */\

int modeset_find_crtc(int fd, drmModeRes *res, drmModeConnector *conn, modeset_dev *dev, modeset_dev **modeset_list)
{
  drmModeEncoder *enc;
  unsigned int i, j;
  int32_t crtc;
  struct modeset_dev *iter;
  
  /* first try the currently conected encoder+crtc */
  
  printf("encoder ** %d\n", conn->encoder_id);
  
  if (conn->encoder_id)
    enc = drmModeGetEncoder(fd, conn->encoder_id);
  else
    enc = NULL;
  
  if (enc)
    {
      if (enc->crtc_id)
	{
	  crtc = enc->crtc_id;
	  for (iter = *modeset_list; iter; iter = iter->next)
	    {
	      if (iter->crtc == crtc)
		{
		  crtc = -1;
		  break;
		}
	    }
	  if (crtc >= 0)
	    {
	      drmModeFreeEncoder(enc);
	      dev->crtc = crtc;
	      return 0;
	    }
	}
      
      drmModeFreeEncoder(enc);
    }

  /* If the connector is not currently bound to an encoder or if the
   * encoder+crtc is already used by another connector (actually unlikely
   * but lets be safe), iterate all other available encoders to find a
   * matching CRTC. */
  
  for (i = 0; i < conn->count_encoders; ++i)
    {
      enc = drmModeGetEncoder(fd, conn->encoders[i]);
      if (!enc)
	{
	  fprintf(stderr, "cannot retrieve encoder %u:%u (%d): %m\n", i, conn->encoders[i], errno);
	  continue;
	}

      /* iterate all global CRTCs */
      for (j = 0; j < res->count_crtcs; ++j)
	{
	  /* check whether this CRTC works with the encoder */
	  if (!(enc->possible_crtcs & (1 << j)))
	    continue;
	  
	  /* check that no other device already uses this CRTC */
	  crtc = res->crtcs[j];
	  for (iter = *modeset_list; iter; iter = iter->next)
	    {
	      if (iter->crtc == crtc)
		{
		  crtc = -1;
		  break;
		}
	    }
	  
	  /* we have found a CRTC, so save it and return */
	  if (crtc >= 0)
	    {
	      drmModeFreeEncoder(enc);
	      dev->crtc = crtc;
	      return 0;
	    }
	}
      
      drmModeFreeEncoder(enc);
    }
  
  fprintf(stderr, "cannot find suitable CRTC for connector %u\n", conn->connector_id);
  return -ENOENT;
}

/* ----------------------------------------------------------------------
   --
   -- modeset_open
   --
   ---------------------------------------------------------------------- */

int modeset_open(int *out, const char *path)
{
  int fd, ret;
  uint64_t has_dumb;
  
  fd = open(path, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      ret = -errno;
      fprintf(stderr, "cannot open '%s': %m\n", path);
      return ret;
    }
  if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb)
    {
      fprintf(stderr, "drm device '%s' does not support dumb buffers\n", path);
      close(fd);
      return -EOPNOTSUPP;
    }
  *out = fd;
  return 0;
}

/* ----------------------------------------------------------------------
   --
   -- mosetdet_setup_dev
   --
   ---------------------------------------------------------------------- */

int modeset_setup_dev(int fd, drmModeRes *res, drmModeConnector *conn, modeset_dev *dev, modeset_dev **modeset_list)
{
  int ret;

  /* check if a monitor is connected */
  if (conn->connection != DRM_MODE_CONNECTED)
    {
      fprintf(stderr, "ignoring unused connector %u\n",conn->connector_id);
      return -ENOENT;
    }

  /* check if there is at least one valid mode */
  if (conn->count_modes == 0)
    {
      fprintf(stderr, "no valid mode for connector %u\n", conn->connector_id);
      return -EFAULT;
    }

  /* copy the mode information into our device structure */
  
  memcpy(&dev->mode, &conn->modes[0], sizeof(dev->mode));
  dev->width = conn->modes[0].hdisplay;
  dev->height = conn->modes[0].vdisplay;
  fprintf(stderr, "mode for connector %u is %ux%u\n", conn->connector_id, dev->width, dev->height);

  /* find a crtc for this connector */
  ret = modeset_find_crtc(fd, res, conn, dev, modeset_list);
  if (ret)
    {
      fprintf(stderr, "no valid crtc for connector %u\n", conn->connector_id);
      return ret;
    }
  return 0;
}

/* ----------------------------------------------------------------------
   --
   -- modeset_prepare
   --
   ---------------------------------------------------------------------- */


int modeset_prepare(int fd, modeset_dev **modeset_list)
{
  drmModeRes *res;
  drmModeConnector *conn;
  struct modeset_dev *dev;
  int ret;
  
  /* retrieve resources */
  res = drmModeGetResources(fd);
  if (!res)
    {
      fprintf(stderr, "cannot retrieve DRM resources (%d): %m\n", errno);
      return -errno;
    }
  
  /* iterate all connectors */
  for (int i = 0; i < res->count_connectors; ++i)
    {
      /* get information for each connector */
      conn = drmModeGetConnector(fd, res->connectors[i]);
      if (!conn)
	{
	  fprintf(stderr, "cannot retrieve DRM connector %u:%u (%d): %m\n", i, res->connectors[i], errno);
	  continue;
	}
      
      /* create a device structure */
      dev = malloc(sizeof(*dev));
      memset(dev, 0, sizeof(*dev));
      dev->conn = conn->connector_id;
      
      /* call helper function to prepare this connector */
      ret = modeset_setup_dev(fd, res, conn, dev, modeset_list);
      if (ret)
	{
	  if (ret != -ENOENT)
	    {
	      errno = -ret;
	      fprintf(stderr, "cannot setup device for connector %u:%u (%d): %m\n",i, res->connectors[i], errno);
	    }
	  free(dev);
	  drmModeFreeConnector(conn);
	  continue;
	}
      
      /* free connector data and link device into global list */
      drmModeFreeConnector(conn);
      dev->next = *modeset_list;
      *modeset_list = dev;
    }
  
  /* free resources again */
  drmModeFreeResources(res);
  return 0;
}

/* ----------------------------------------------------------------------
   --
   -- modeset_cleanup
   --
   ---------------------------------------------------------------------- */

void modeset_cleanup(int fd, modeset_dev **modeset_list)
{
  struct modeset_dev *iter;
  struct drm_mode_destroy_dumb dreq;
  
  while (*modeset_list)
    {
      /* remove from global list */
      iter = *modeset_list;
      *modeset_list = iter->next;
      
    /* restore saved CRTC configuration */
      drmModeSetCrtc(fd,
		     iter->saved_crtc->crtc_id,
		     iter->saved_crtc->buffer_id,
		     iter->saved_crtc->x,
		     iter->saved_crtc->y,
		     &iter->conn,
		     1,
		     &iter->saved_crtc->mode);
      drmModeFreeCrtc(iter->saved_crtc);
    
      munmap(iter->map, iter->size);
      drmModeRmFB(fd, iter->fb);
    
      memset(&dreq, 0, sizeof(dreq));
      dreq.handle = iter->handle;
      drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
      free(iter);
    }
}

/* ----------------------------------------------------------------------
   --
   -- next_color
   --
   ---------------------------------------------------------------------- */

uint8_t next_color(bool *up, uint8_t cur, unsigned int mod)
{
  uint8_t next;
  
  next = cur + (*up ? 1 : -1) * (rand() % mod);
  if ((*up && next < cur) || (!*up && next > cur))
    {
      *up = !*up;
      next = cur;
    }
  return next;
}

/* ----------------------------------------------------------------------
   --
   -- gegl_draw
   --
   ---------------------------------------------------------------------- */

void gegl_draw(modeset_dev *iter, uint8_t r, uint8_t g, uint8_t b)
{
  printf("draw %d × %d × %d\n", iter->width, iter->height, iter->stride);
  
  double x = 0.0;
  double y = 0.0;
  double w = iter->width;
  double h = iter->height;
  
  GeglNode *graph = gegl_node_new();
  GeglPath *path = gegl_path_new();
  gegl_path_append(path, 'M', x, y);
  gegl_path_append(path, 'L', x + w, y);
  gegl_path_append(path, 'L', x + w, y + h);
  gegl_path_append(path, 'L', x, y + h);
  gegl_path_append(path, 'L', x, y);
  gegl_path_append(path, 'z');
  
  GeglColor *c = gegl_color_new(0);
  GeglColor *black = gegl_color_new(0);
  
  gegl_color_set_rgba(c, (double)r/256.0, (double)g/256.0, (double)b/256.0, 1.0);
  gegl_color_set_rgba(black, 1.0, 0.0, 0.0, 1.0);
  
  GeglNode *stroke = gegl_node_new_child(graph, "operation", "gegl:path",
					 "d", path,
					 "fill", c,
					 "fill-opacity", 0.5,
					 "stroke", black,
					 "stroke-width", 10.0,
					 "stroke-hardness", 1.0,
					 0);
  
  const Babl *format = babl_format_new (babl_model ("R'aG'aB'aA"),
					babl_type ("u8"),
					babl_component ("B'a"),
					babl_component ("G'a"),
					babl_component ("R'a"),
					babl_component ("A"),
					0);
  GeglRectangle size;
  size.x = size.y = 0;
  size.width = iter->width;
  size.height = iter->height;
  
  gegl_node_blit (stroke,
		  1.0 /*scale*/,
		  &size,
		  format,
		  iter->map,
		  GEGL_AUTO_ROWSTRIDE,
		  GEGL_BLIT_DEFAULT);
}

/* ----------------------------------------------------------------------
   --
   -- cairo_draw
   --
   ---------------------------------------------------------------------- */

void cairo_draw(modeset_dev *iter, int r, int g, int b)
{
  printf("draw %d × %d × %d\n", iter->width, iter->height, iter->stride);
  
  double x = 0.0;
  double y = 0.0;
  double w = iter->width;
  double h = iter->height;

  cairo_surface_t *surface = cairo_image_surface_create_for_data(iter->map, CAIRO_FORMAT_ARGB32, iter->width, iter->height, cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, iter->width));
  cairo_t *cr = cairo_create(surface);
  cairo_set_source_rgb(cr, (double)r/256.0, (double)g/256.0, (double)b/256.0);
  cairo_rectangle(cr, x, y, w, h);
  cairo_fill(cr);
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}

  
/* ----------------------------------------------------------------------
   --
   -- modeset_draw
   --
   ---------------------------------------------------------------------- */

void modeset_draw(modeset_dev **modeset_list)
{
  uint8_t r, g, b;
  bool r_up, g_up, b_up;
  unsigned int off;
  struct modeset_dev *iter;
  
  srand(time(NULL));
  r = rand() % 0xff;
  g = rand() % 0xff;
  b = rand() % 0xff;
  r_up = g_up = b_up = true;
  
  for (int i = 0; i < 50; ++i)
    {  
      r = next_color(&r_up, r, 20);
      g = next_color(&g_up, g, 10);
      b = next_color(&b_up, b, 5);

      
      for (iter = *modeset_list; iter; iter = iter->next)
	{
	  gegl_draw(iter, r, g, b);
#ifdef X
	  for (int j = 0; j < iter->height; ++j)
	    {
	      for (int k = 0; k < iter->width; ++k)
		{
		  off = iter->stride * j + k * 4;
		  *(uint32_t*)&iter->map[off] = (r << 16) | (g << 8) | b;
		}
	    }
#endif
	}
      usleep(100000);
    }
}

/* ----------------------------------------------------------------------
   --
   -- modeset
   --
   ---------------------------------------------------------------------- */

int modeset(const char *path)
{
  int ret, fd;
  
  /* open the DRM device */
  ret = modeset_open(&fd, path);
  if (ret)
    goto out_return;

  /* prepare all connectors and CRTCs */

  modeset_dev *modeset_list = 0;
  
  ret = modeset_prepare(fd, &modeset_list);
  if (ret)
    goto out_close;

  /* perform actual modesetting on each found connector+CRTC */
  for (modeset_dev *iter = modeset_list; iter; iter = iter->next)
    {
      iter->saved_crtc = drmModeGetCrtc(fd, iter->crtc);

      /* Modeset FB creation to here */
      ret = modeset_create_fb(fd, iter);
      if (ret)
	{
	  fprintf(stderr, "cannot create framebuffer for connector %u\n", iter->conn);
	  return ret;
	}      
      ret = drmModeSetCrtc(fd, iter->crtc, iter->fb, 0, 0, &iter->conn, 1, &iter->mode);
      if (ret)
	fprintf(stderr, "cannot set CRTC for connector %u (%d): %m\n", iter->conn, errno);
    }
  modeset_draw(&modeset_list);  
  modeset_cleanup(fd, &modeset_list);
  ret = 0;

  
 out_close:
  close(fd);
 out_return:
  if (ret)
    {
      errno = -ret;
      fprintf(stderr, "modeset failed with error %d: %m\n", errno);
    }
  else
    {
      fprintf(stderr, "exiting\n");
    }
  return ret;
}  

/* ----------------------------------------------------------------------
   --
   -- gbm_draw
   --
   ---------------------------------------------------------------------- */

void gbm_draw(modeset_dev **modeset_list)
{
  uint8_t r, g, b;
  bool r_up, g_up, b_up;
  unsigned int off;
  struct modeset_dev *iter;
  
  srand(time(NULL));
  r = rand() % 0xff;
  g = rand() % 0xff;
  b = rand() % 0xff;
  r_up = g_up = b_up = true;
  
  for (int i = 0; i < 50; ++i)
    {  
      r = next_color(&r_up, r, 20);
      g = next_color(&g_up, g, 10);
      b = next_color(&b_up, b, 5);
  
      for (iter = *modeset_list; iter; iter = iter->next)
	{
	  printf("draw %d × %d × %d\n", iter->width, iter->height, iter->stride);

	  void *gbo_mapping = NULL;
	  uint32_t dst_stride = 0;
	  iter->map = gbm_bo_map(iter->bo, 0, 0, iter->width, iter->height, GBM_BO_TRANSFER_READ_WRITE, &dst_stride, &gbo_mapping);
	  assert(iter->map);
	  
	  //	  gegl_draw(iter, r, g, b);
	  cairo_draw(iter, r,g,b );

#ifdef X
	  for (int j = 0; j < iter->height; ++j)
	    {
	      for (int k = 0; k < iter->width; ++k)
		{
		  off = iter->stride * j + k * 4;
		  *(uint32_t*)&iter->map[off] = (r << 16) | (g << 8) | b;
		}
	    }
#endif
	  gbm_bo_unmap(iter->bo, gbo_mapping);

	}
      usleep(100000);
    }
}

/* ----------------------------------------------------------------------
   --
   -- gbm
   --
   ---------------------------------------------------------------------- */

int gbm(const char *path)
{
  int ret, fd;
  
  /* open the DRM device */
  ret = modeset_open(&fd, path);
  if (ret)
    goto out_return;

  /* prepare all connectors and CRTCs */
  
  modeset_dev *modeset_list = 0;
  ret = modeset_prepare(fd, &modeset_list);
  if (ret)
    goto out_close;

  struct gbm_device *gbm_dev;
  
  if(!(gbm_dev = gbm_create_device(fd)))
    {
      fprintf(stderr, "can't create gbm device: %s\n", strerror(errno));
      return 1;
    }
  printf("Using GBM backend device %s\n", gbm_device_get_backend_name(gbm_dev));

  if (!gbm_device_is_format_supported(gbm_dev, GBM_FORMAT_ARGB8888, GBM_BO_USE_SCANOUT|GBM_BO_USE_RENDERING|GBM_BO_USE_WRITE))
    {
      fprintf(stderr, "Format not supported\n");
      exit(1);
    }

  //  uint64_t modifier = DRM_FORMAT_MOD_LINEAR;

  for (modeset_dev *iter = modeset_list; iter; iter = iter->next)
    {
      iter->saved_crtc = drmModeGetCrtc(fd, iter->crtc);
      
      //      struct gbm_surface *surface = gbm_surface_create_with_modifiers(gbm_dev, iter->width, iter->height, GBM_FORMAT_ARGB8888, &modifier, 1);
      //      assert(surface);
      
      iter->bo = gbm_bo_create(gbm_dev, iter->width, iter->height, GBM_FORMAT_ARGB8888, GBM_BO_USE_RENDERING|GBM_BO_USE_SCANOUT|GBM_BO_USE_LINEAR);
      assert(iter->bo);

       for (int i = 0; i < gbm_bo_get_plane_count(iter->bo); i++)
	 {
	   printf("plane %d stride %d offset %d\n", i, gbm_bo_get_stride_for_plane(iter->bo, i), gbm_bo_get_offset(iter->bo, i));
	 }
      //      struct gbm_bo *bo = gbm_surface_lock_front_buffer(surface);

      uint32_t handle = gbm_bo_get_handle(iter->bo).u32;
      uint32_t pitch = gbm_bo_get_stride(iter->bo);
      iter->stride = pitch;
      uint32_t fb;
      
      printf("creating surface %d × %d × %d\n", iter->width, iter->height, pitch);
      int ret = drmModeAddFB(fd, iter->width, iter->height, 24, 32, pitch, handle, &fb); 
      ret = drmModeSetCrtc(fd, iter->crtc, fb, 0, 0, &iter->conn, 1, &iter->mode);
    }
  gbm_draw(&modeset_list);  
  modeset_cleanup(fd, &modeset_list);

  return 0;
 out_close:
  close(fd);
 out_return:
  if (ret)
    {
      errno = -ret;
      fprintf(stderr, "modeset failed with error %d: %m\n", errno);
    }
  else
    {
      fprintf(stderr, "exiting\n");
    }
  return ret;
}

/* ----------------------------------------------------------------------
   --
   -- main
   --
   ---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  static struct option long_options[] = {
    {"gbm",     no_argument,   0,  'g' },
    {"kms",     no_argument,   0,  'k' },
    {0,         0,                 0,  0 }
  };

  gegl_init(&argc, &argv);

  int (*func)(const char*) = 0;
  
  while(1)
    {
      int option_index = 0;
      int c = getopt_long(argc, argv, "", long_options, &option_index);
        if (c == -1)
            break;
	switch(c)
	  {
	  case 'g':
	    func = &gbm;
	    break;
	    
	  case 'k':
	    func = &modeset;
	    break;
	  }
    }
  const char *card;
  if (optind < argc)
    card = argv[optind];
  else
    card = "/dev/dri/card0";
  
  fprintf(stderr, "using card '%s'\n", card);

  if (func)
    return (*func)(card);
  return 0;
}
