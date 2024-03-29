/* ----------------------------------------------------------------------
--
-- original source is https://github.com/eyelash/tutorials/blob/master/drm-gbm.c
--
-- 2022-02-03: Ross Alexander
--   Added extra bits to find additional crtc
--
---------------------------------------------------------------------- */

// gcc -o drm-gbm drm-gbm.c -ldrm -lgbm -lEGL -lGL -I/usr/include/libdrm

// general documentation: man drm

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <drm_fourcc.h>
#include <gbm.h>

#include <EGL/egl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define EXIT(msg) { fputs (msg, stderr); exit (EXIT_FAILURE); }

typedef struct example_t {
  int device;
  struct gbm_device *gbm_device;
  struct gbm_surface *gbm_surface;
  EGLSurface egl_surface;
  EGLDisplay display;
  EGLContext context;
  uint32_t connector_id;
  drmModeModeInfo mode_info;
  drmModeCrtc *crtc;
  struct gbm_bo *previous_bo;
  uint32_t previous_fb;
} example_t;

/* ----------------------------------------------------------------------
--
-- find_connector
--
---------------------------------------------------------------------- */

static drmModeConnector *find_connector(int device, drmModeRes *resources)
{
  // iterate the connectors
  for (int i = 0; i < resources->count_connectors; i++)
    {
      drmModeConnector *connector = drmModeGetConnector(device, resources->connectors[i]);
    // pick the first connected connector
      if ((connector->connection == DRM_MODE_CONNECTED) && (connector->count_modes > 0))
	{
	  return connector;
	}
      drmModeFreeConnector(connector);
    }
  // no connector found
  return NULL;
}

/* ----------------------------------------------------------------------
--
-- find_encoder
--
---------------------------------------------------------------------- */

static drmModeEncoder *find_encoder(int device, drmModeRes *resources, drmModeConnector *connector)
{
  drmModeEncoder *enc;
  if (connector->encoder_id)
    {
      return drmModeGetEncoder(device, connector->encoder_id);
    }
  for (int i = 0; i < connector->count_encoders; ++i)
    {
      enc = drmModeGetEncoder(device, connector->encoders[i]);
      if (!enc)
	{
	  fprintf(stderr, "cannot retrieve encoder %u:%u (%d): %m\n", i, connector->encoders[i], errno);
	  continue;
	}
    }
  if (enc) return enc;
  // no encoder found
  return NULL;
}

/* ----------------------------------------------------------------------
--
-- find_display_configuration
--
---------------------------------------------------------------------- */

static void find_display_configuration(example_t *example)
{
  drmModeRes *resources = drmModeGetResources(example->device);

  // find a connector
  drmModeConnector *connector = find_connector(example->device, resources);
  if (!connector) EXIT ("no connector found\n");
  // save the connector_id
  example->connector_id = connector->connector_id;

  // save the first mode
  example->mode_info = connector->modes[0];
  printf ("resolution: %ix%i\n", example->mode_info.hdisplay, example->mode_info.vdisplay);

  // find an encoder
  drmModeEncoder *encoder = find_encoder (example->device, resources, connector);
  if (!encoder) EXIT ("no encoder found\n");
  // find a CRTC

  int crtc_id = -1;
  for (int j = 0; j < resources->count_crtcs; j++)
    {
      if (!(encoder->possible_crtcs & (1 << j)))
	continue;
      crtc_id = resources->crtcs[j];
      if (crtc_id >= 0)
	{
	  break;
	}
    }
  assert(crtc_id >= 0);
  example->crtc = drmModeGetCrtc(example->device, crtc_id);
  drmModeFreeEncoder(encoder);
  drmModeFreeConnector(connector);
  drmModeFreeResources(resources);
}

/* ----------------------------------------------------------------------
--
-- setup_opengl
--
---------------------------------------------------------------------- */

static void setup_opengl (example_t *example)
{
  example->gbm_device = gbm_create_device(example->device);
  assert(example->gbm_device);

  printf("Using GBM backend device %s\n", gbm_device_get_backend_name(example->gbm_device));
  
  example->display = eglGetDisplay(example->gbm_device);
  assert(example->display);

  eglInitialize(example->display, NULL, NULL);
  
  // create an OpenGL context
  assert(eglBindAPI(EGL_OPENGL_API) == EGL_TRUE);

  EGLint attributes[] = {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_NONE}
    ;
  EGLConfig* configs;
  EGLint valid_config = -1, num_config = 0;
  
  eglChooseConfig(example->display, attributes, 0, 0, &num_config);
  printf("%d EGL configurations found.\n", num_config);

  configs = calloc(sizeof(EGLConfig), num_config);
  eglChooseConfig(example->display, attributes, configs, num_config, &num_config);

  printf("%d EGL configurations found.\n", num_config);

  for (int i = 0; i < num_config; i++)
    {
      EGLint gbm_format;
      if (!eglGetConfigAttrib(example->display, configs[i], EGL_NATIVE_VISUAL_ID, &gbm_format))
	{
	  abort();
	}
      
      if (gbm_format == GBM_FORMAT_ARGB8888)
	{
	  valid_config = i;
	  continue;
	}
    }
  if (valid_config < 0)
    {
      printf("No valid configuration found.\n");
      abort();
    }

  example->context = eglCreateContext (example->display, configs[valid_config], EGL_NO_CONTEXT, NULL);

  assert(example->context);
  
  if (!gbm_device_is_format_supported(example->gbm_device, DRM_FORMAT_ARGB8888, GBM_BO_USE_SCANOUT|GBM_BO_USE_RENDERING))
    {
      fprintf(stderr, "Format not supported\n");
      exit(1);
    }
  
  uint64_t modifier = DRM_FORMAT_MOD_LINEAR;
  
  // create the GBM and EGL surface

  example->gbm_surface = gbm_surface_create_with_modifiers(example->gbm_device, example->mode_info.hdisplay, example->mode_info.vdisplay, GBM_FORMAT_ARGB8888, &modifier, 1);
  assert(example->gbm_surface);

  example->egl_surface = eglCreateWindowSurface (example->display, configs[valid_config], example->gbm_surface, NULL);
  assert(example->egl_surface);

  eglMakeCurrent(example->display, example->egl_surface, example->egl_surface, example->context);
}

/* ----------------------------------------------------------------------
--
-- clean_up
--
---------------------------------------------------------------------- */

static void clean_up (example_t *example)
{
  // set the previous crtc
  drmModeSetCrtc(example->device, example->crtc->crtc_id, example->crtc->buffer_id, example->crtc->x, example->crtc->y, &example->connector_id, 1, &example->crtc->mode);
  drmModeFreeCrtc(example->crtc);
  
  if (example->previous_bo)
    {
      drmModeRmFB (example->device, example->previous_fb);
      gbm_surface_release_buffer (example->gbm_surface, example->previous_bo);
    }
  
  eglDestroySurface (example->display, example->egl_surface);
  gbm_surface_destroy (example->gbm_surface);
  eglDestroyContext (example->display, example->context);
  eglTerminate (example->display);
  gbm_device_destroy (example->gbm_device);
}


/* ----------------------------------------------------------------------
--
-- swap_buffers
--
---------------------------------------------------------------------- */

static void swap_buffers (example_t *example)
{
  eglSwapBuffers(example->display, example->egl_surface);

  struct gbm_bo *bo = gbm_surface_lock_front_buffer(example->gbm_surface);

  uint32_t handle = gbm_bo_get_handle (bo).u32;
  uint32_t pitch = gbm_bo_get_stride (bo);
  uint32_t fb;

  int ret = drmModeAddFB(example->device, example->mode_info.hdisplay, example->mode_info.vdisplay, 24, 32, pitch, handle, &fb);
  
  drmModeSetCrtc(example->device, example->crtc->crtc_id, fb, 0, 0, &example->connector_id, 1, &example->mode_info);
  
  if (example->previous_bo)
    {
      drmModeRmFB(example->device, example->previous_fb);
      gbm_surface_release_buffer(example->gbm_surface, example->previous_bo);
    }
  example->previous_bo = bo;
  example->previous_fb = fb;
}

/* ----------------------------------------------------------------------
   --
   -- draw
   --
   ---------------------------------------------------------------------- */

static void draw (example_t *example, float progress)
{
  glClearColor(1.0f-progress, progress, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  //  glRotatef(progress, 0, 0, 0);
  
  //  glClear (GL_COLOR_BUFFER_BIT);
  glBegin (GL_TRIANGLES);
  glColor3f (1.0, 0.0, 0.0);
  glVertex2f (5.0, 5.0);
  glColor3f (0.0, 1.0, 0.0);
  glVertex2f (25.0, 5.0);
  glColor3f (0.0, 0.0, 1.0);
  glVertex2f (5.0, 25.0);
  glEnd();
  //  glFlush ();
  swap_buffers(example);
  //  sleep(1);
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  const char *path = "/dev/dri/card0";
  example_t *example = calloc(sizeof(example_t), 1);
  example->device = open(path, O_RDWR|O_CLOEXEC);
  if (example->device < 0)
    {
      fprintf(stderr, "cannot open %s\n", path);
      exit(1);
    }

  find_display_configuration(example);
  setup_opengl(example);

  int w = example->mode_info.hdisplay;
  int h = example->mode_info.vdisplay;
  
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel (GL_SMOOTH);
   glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   if (w <= h)
      gluOrtho2D (0.0, 30.0, 0.0, 30.0 * (GLfloat) h/(GLfloat) w);
   else
      gluOrtho2D (0.0, 30.0 * (GLfloat) w/(GLfloat) h, 0.0, 30.0);
   glMatrixMode(GL_MODELVIEW);
  
   for (int i = 0; i < 600; i++)
     draw (example, i / 600.0f);
   
  clean_up(example);
  close (example->device);
  return 0;
}
