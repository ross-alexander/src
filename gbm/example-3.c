#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <xf86drm.h>
#include <unistd.h>
#include <xf86drmMode.h>

#include <GLES3/gl31.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define MAX_DEVICES 16
#define _EXIT(str, n) {printf(str);exit(n);}

static PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT = NULL;
static PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT = NULL;
static PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = NULL;
static PFNEGLGETOUTPUTLAYERSEXTPROC eglGetOutputLayersEXT = NULL;
static PFNEGLCREATESTREAMKHRPROC eglCreateStreamKHR = NULL;
static PFNEGLDESTROYSTREAMKHRPROC eglDestroyStreamKHR = NULL;
static PFNEGLSTREAMCONSUMEROUTPUTEXTPROC eglStreamConsumerOutputEXT = NULL;
static PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC eglCreateStreamProducerSurfaceKHR = NULL;

EGLDisplay eglDisplay = EGL_NO_DISPLAY;
EGLSurface eglSurface = EGL_NO_SURFACE;
EGLContext eglContext = EGL_NO_CONTEXT;

int plane = -1;
int xsurfsize = 0, ysurfsize = 0;
uint32_t drm_crtc_id, drm_plane_id;

// Extension checking utility
static bool CheckExtension(const char* exts, const char* ext)
{
	int extLen = (int)strlen(ext);
	const char* end = exts + strlen(exts);

	while (exts < end) {
		while (*exts == ' ')
			exts++;

		int n = strcspn(exts, " ");
		if ((extLen == n) && (strncmp(ext, exts, n) == 0))
			return true;

		exts += n;
	}
	return false;
}

void load_func() {
	// Load extension function pointers.
	eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");
	eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC)eglGetProcAddress("eglQueryDeviceStringEXT");
	eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
	eglGetOutputLayersEXT = (PFNEGLGETOUTPUTLAYERSEXTPROC)eglGetProcAddress("eglGetOutputLayersEXT");
	eglCreateStreamKHR = (PFNEGLCREATESTREAMKHRPROC)eglGetProcAddress("eglCreateStreamKHR");
	eglDestroyStreamKHR = (PFNEGLDESTROYSTREAMKHRPROC)eglGetProcAddress("eglDestroyStreamKHR");
	eglStreamConsumerOutputEXT = (PFNEGLSTREAMCONSUMEROUTPUTEXTPROC)eglGetProcAddress("eglStreamConsumerOutputEXT");
	eglCreateStreamProducerSurfaceKHR = (PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC)eglGetProcAddress("eglCreateStreamProducerSurfaceKHR");
	if (!eglQueryDevicesEXT ||
		!eglQueryDeviceStringEXT ||
		!eglGetPlatformDisplayEXT ||
		!eglGetOutputLayersEXT ||
		!eglCreateStreamKHR ||
		!eglDestroyStreamKHR ||
		!eglStreamConsumerOutputEXT ||
		!eglCreateStreamProducerSurfaceKHR)
		_EXIT("Missing required function(s)\n", 2);

	printf("Loaded extension functions\n");
}

void graphics_set_drm(EGLDeviceEXT egl_dev) {
	bool set_mode = false;
	int crtc = -1;

	int xoffset = 0, yoffset = 0;
	int xmodesize = 0, ymodesize = 0;
	int bounce = 0;

	const char* drm_name;
	int drm_fd;
	uint32_t drm_conn_id, drm_enc_id;
	
	uint32_t crtc_mask;
	drmModeRes* drm_res_info = NULL;
	drmModePlaneRes* drm_plane_res_info = NULL;
	drmModeCrtc* drm_crtc_info = NULL;
	drmModeConnector* drm_conn_info = NULL;
	drmModeEncoder* drm_enc_info = NULL;
	drmModePlane* drm_plane_info = NULL;
	int drm_mode_index = 0;

	// Obtain and open DRM device file
	drm_name = eglQueryDeviceStringEXT(egl_dev, EGL_DRM_DEVICE_FILE_EXT);
	if (!drm_name)
		_EXIT("Couldn't obtain device file \n", 3);

	printf("Device file: %s\n", drm_name);
	drm_fd = drmOpen(drm_name, NULL);
	if (drm_fd == -1)
		_EXIT("Couldn't open device file\n", 3);


	// Obtain DRM-KMS resources
	drm_res_info = drmModeGetResources(drm_fd);
	if (!drm_res_info)
		_EXIT("Couldn't obtain DRM-KMS resources\n", 3);

	printf("Obtained device information\n");

	// If a specific crtc was requested, make sure it exists
	if (crtc >= drm_res_info->count_crtcs)
		_EXIT("Requested crtc index exceeds count\n", 4);

	crtc_mask = (crtc >= 0) ? (1 << crtc) : ((1 << drm_res_info->count_crtcs) - 1);

	// If drawing to a plane is requested, obtain the plane info
	if (plane >= 0) {
		drm_plane_res_info = drmModeGetPlaneResources(drm_fd);
		if (!drm_plane_res_info)
			_EXIT("Unable to obtain plane resource list\n", 5);

		if (plane >= drm_plane_res_info->count_planes)
			_EXIT("Requested plane index  exceeds count \n", 5);

		drm_plane_id = drm_plane_res_info->planes[plane];
		drm_plane_info = drmModeGetPlane(drm_fd, drm_plane_id);
		if (!drm_plane_info)
			_EXIT("Unable to obtain info for plane\n", 5);

		crtc_mask &= drm_plane_info->possible_crtcs;
		if (!crtc_mask)
			_EXIT("Requested crtc and plane not compatible\n", 5);

		printf("Obtained plane information\n");
	}

	int conn = 0;
	// Query info for requested connector
	for (conn = 0; conn < drm_res_info->count_connectors; ++conn)
	{
		drm_conn_id = drm_res_info->connectors[conn];
		drm_conn_info = drmModeGetConnector(drm_fd, drm_conn_id);
		if (drm_conn_info != NULL)
		{
			printf("connector %d found\n", drm_conn_info->connector_id);
			if (drm_conn_info->connection == DRM_MODE_CONNECTED && drm_conn_info->count_modes > 0)
				break;

			drmModeFreeConnector(drm_conn_info);
		}
	}

	if (conn == drm_res_info->count_connectors)
		_EXIT("No active connectors found\n", 6);

	printf("Obtained connector information\n");

	// If there is already an encoder attached to the connector, choose
	//   it unless not compatible with crtc/plane
	drm_enc_id = drm_conn_info->encoder_id;
	drm_enc_info = drmModeGetEncoder(drm_fd, drm_enc_id);
	if (drm_enc_info) {
		if (!(drm_enc_info->possible_crtcs & crtc_mask)) {
			drmModeFreeEncoder(drm_enc_info);
			drm_enc_info = NULL;
		}
	}

	// If we didn't have a suitable encoder, find one
	if (!drm_enc_info) {
		int i = 0;
		for (i = 0; i < drm_conn_info->count_encoders; ++i) {
			drm_enc_id = drm_conn_info->encoders[i];
			drm_enc_info = drmModeGetEncoder(drm_fd, drm_enc_id);
			if (drm_enc_info) {
				if (crtc_mask & drm_enc_info->possible_crtcs) {
					crtc_mask &= drm_enc_info->possible_crtcs;
					break;
				}
				drmModeFreeEncoder(drm_enc_info);
				drm_enc_info = NULL;
			}
		}
		if (i == drm_conn_info->count_encoders)
			_EXIT("Unable to find suitable encoder\n", 7);
	}
	printf("Obtained encoder information\n");

	// Select a suitable crtc. Give preference to any that's already
	//   attached to the encoder. (Could make this more sophisticated
	//   by finding one not already bound to any other encoders. But
	//   this is just a basic test, so we don't really care that much.)
	assert(crtc_mask);
	for (int i = 0; i < drm_res_info->count_crtcs; ++i) {
		if (crtc_mask & (1 << i)) {
			drm_crtc_id = drm_res_info->crtcs[i];
			if (drm_res_info->crtcs[i] == drm_enc_info->crtc_id) {
				break;
			}
		}
	}

	// Query info for crtc
	drm_crtc_info = drmModeGetCrtc(drm_fd, drm_crtc_id);
	if (!drm_crtc_info)
		_EXIT("Unable to obtain info for crtc\n", 4);

	printf("Obtained crtc information\n");

	// If dimensions are specified and not using a plane, find closest mode
	if ((xmodesize || ymodesize) && (plane < 0)) {

		// Find best fit among available modes
		int best_index = 0;
		int best_fit = 0x7fffffff;
		for (int i = 0; i < drm_conn_info->count_modes; ++i) {
			drmModeModeInfoPtr mode = drm_conn_info->modes + i;
			int fit = 0;

			if (xmodesize)
				fit += abs((int)mode->hdisplay - xmodesize) * (int)mode->vdisplay;

			if (ymodesize)
				fit += abs((int)mode->vdisplay - ymodesize) * (int)mode->hdisplay;

			if (fit < best_fit) {
				best_index = i;
				best_fit = fit;
			}
		}

		// Choose this size/mode
		drm_mode_index = best_index;
		xmodesize = (int)drm_conn_info->modes[best_index].hdisplay;
		ymodesize = (int)drm_conn_info->modes[best_index].vdisplay;
	}

	// We'll only set the mode if we have to. This hopefully allows
	//   multiple instances of this application to run, writing to
	//   separate planes of the same display, as long as they don't
	//   specifiy incompatible settings.
	if ((drm_conn_info->encoder_id != drm_enc_id) ||
		(drm_enc_info->crtc_id != drm_crtc_id) ||
		!drm_crtc_info->mode_valid ||
		((plane < 0) && xmodesize && (xmodesize != (int)drm_crtc_info->mode.hdisplay)) ||
		((plane < 0) && ymodesize && (ymodesize != (int)drm_crtc_info->mode.vdisplay))) {
		set_mode = true;
	}

	// If dimensions haven't been specified, figure out good values to use
	if (!xmodesize || !ymodesize) {

		// If mode requires reset, just pick the first one available
		//   from the connector
		if (set_mode) {
			xmodesize = (int)drm_conn_info->modes[0].hdisplay;
			ymodesize = (int)drm_conn_info->modes[0].vdisplay;
		}

		// Otherwise get it from the current crtc settings
		else {
			xmodesize = (int)drm_crtc_info->mode.hdisplay;
			ymodesize = (int)drm_crtc_info->mode.vdisplay;
		}
	}
	printf("Determine mode settings\n");

	// If surf size is unspecified, default to fullscreen normally
	// or to 1/4 fullscreen if in animated bounce mode.
	if (!xsurfsize || !ysurfsize) {
		if (bounce) {
			xsurfsize = xmodesize / 2;
			ysurfsize = ymodesize / 2;
		}
		else {
			xsurfsize = xmodesize;
			ysurfsize = ymodesize;
		}
	}
	printf("Determine surface size\n");

	// If necessary, set the mode
	if (set_mode) {
		drmModeSetCrtc(drm_fd, drm_crtc_id, -1, 0, 0, &drm_conn_id, 1, drm_conn_info->modes + drm_mode_index);
		printf("Set mode\n");
	}

	// If plane is in use, set it
	if (plane >= 0) {
		drmModeSetPlane(drm_fd, drm_plane_id, drm_crtc_id, -1, 0,
			xoffset, yoffset, xsurfsize, ysurfsize,
			0, 0, xsurfsize << 16, ysurfsize << 16);
		printf("Set plane configuration\n");
	}
}

void graphics_set_egl(EGLDeviceEXT egl_dev) {
	int fifo = 0;
	EGLOutputLayerEXT egl_lyr;
	EGLConfig egl_cfg;
	EGLStreamKHR egl_str;
	EGLint major, minor;
	// Obtain and initialize EGLDisplay
	eglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, (void*)egl_dev, NULL);
	if (eglDisplay == EGL_NO_DISPLAY)
		_EXIT("Couldn't obtain EGLDisplay for device\n", 8);

	if (!eglInitialize(eglDisplay, &major, &minor))
		_EXIT("Couldn't initialize EGLDisplay (error)\n", eglGetError());

	printf("Obtained EGLDisplay\n");

	// Check for stream_consumer_egloutput + output_drm support
	const char* dpy_exts = eglQueryString(eglDisplay, EGL_EXTENSIONS);
	const char* dev_exts = eglQueryDeviceStringEXT(egl_dev, EGL_EXTENSIONS);

	if (!CheckExtension(dpy_exts, "EGL_EXT_output_base"))
		_EXIT("Missing required extension: EGL_EXT_output_base\n", 2);

	if (!CheckExtension(dev_exts, "EGL_EXT_device_drm"))
		_EXIT("Missing required extension: EGL_EXT_device_drm\n", 2);

	if (!CheckExtension(dpy_exts, "EGL_EXT_output_drm"))
		_EXIT("Missing required extension: EGL_EXT_output_drm\n", 2);

	if (!CheckExtension(dpy_exts, "EGL_EXT_stream_consumer_egloutput"))
		_EXIT("Missing required extension: EGL_EXT_stream_consumer_egloutput\n", 2)

		// Choose a config and create a context
		EGLint cfg_attr[] = {
			EGL_SURFACE_TYPE, EGL_STREAM_BIT_KHR,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_ALPHA_SIZE, 1,
			EGL_NONE
	};

	int n;
	if (!eglChooseConfig(eglDisplay, cfg_attr, &egl_cfg, 1, &n) || !n)
		_EXIT("Unable to obtain config that supports stream rendering (error)\n", eglGetError());

	EGLint ctx_attr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	eglBindAPI(EGL_OPENGL_ES_API);

	eglContext = eglCreateContext(eglDisplay, egl_cfg, EGL_NO_CONTEXT, ctx_attr);
	if (eglContext == EGL_NO_CONTEXT)
		_EXIT("Unable to create context (error)\n", eglGetError());

	printf("Obtained EGLConfig and EGLContext\n");

	// Get the layer for this crtc/plane
	EGLAttrib layer_attr[] = { EGL_NONE, EGL_NONE, EGL_NONE };
	if (plane >= 0) {
		layer_attr[0] = EGL_DRM_PLANE_EXT;
		layer_attr[1] = (EGLAttrib)drm_plane_id;
	}
	else {
		layer_attr[0] = EGL_DRM_CRTC_EXT;
		layer_attr[1] = (EGLAttrib)drm_crtc_id;
	}
	if (!eglGetOutputLayersEXT(eglDisplay, layer_attr, &egl_lyr, 1, &n) || !n)
		_EXIT("Unable to obtain EGLOutputLayer ", 10);

	printf("Obtained EGLOutputLayer\n");

	// Create a stream and connect to the output
	EGLint stream_attr[] = { EGL_STREAM_FIFO_LENGTH_KHR, fifo, EGL_NONE };
	egl_str = eglCreateStreamKHR(eglDisplay, stream_attr);
	if (egl_str == EGL_NO_STREAM_KHR)
		_EXIT("Unable to create stream (error)\n", eglGetError());

	if (!eglStreamConsumerOutputEXT(eglDisplay, egl_str, egl_lyr))
		_EXIT("Unable to connect stream (error)\n", eglGetError());

	// Create a surface to feed the stream
	EGLint srf_attr[] = { EGL_WIDTH, xsurfsize, EGL_HEIGHT, ysurfsize, EGL_NONE };
	eglSurface = eglCreateStreamProducerSurfaceKHR(eglDisplay, egl_cfg, egl_str, srf_attr);

	if (eglSurface == EGL_NO_SURFACE)
		_EXIT("Unable to create rendering surface (error)\n", eglGetError());

	printf("Bound layer to rendering surface\n");

	// Make current
	if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
		_EXIT("Unable to make context/surface current (error)\n", eglGetError());

	EGLint Context_RendererType;
	eglQueryContext(eglDisplay, eglContext, EGL_CONTEXT_CLIENT_TYPE, &Context_RendererType);
}

int graphics_setup_window(int xpos, int ypos, int width, int height)
{
	load_func();

	// Query device
	int device = 0;
	EGLDeviceEXT egl_devs[MAX_DEVICES];
	int n;
	if (!eglQueryDevicesEXT(device + 1, egl_devs, &n) || (n <= device))
		_EXIT("Requested device index not found\n", 2);

	EGLDeviceEXT egl_dev = egl_devs[device];
	graphics_set_drm(egl_dev);
	graphics_set_egl(egl_dev);
	return 1;
}

void graphics_swap_buffers()
{
	eglSwapBuffers(eglDisplay, eglSurface);
}

void graphics_close_window()
{

	if (eglDisplay != EGL_NO_DISPLAY)
	{
		eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		if (eglContext != EGL_NO_CONTEXT)
			eglDestroyContext(eglDisplay, eglContext);

		if (eglSurface != EGL_NO_SURFACE)
			eglDestroySurface(eglDisplay, eglSurface);

		eglTerminate(eglDisplay);
	}

	printf("Released display resources\n");
}

static void InitGraphicsState() {
	char* GL_version = (char*)glGetString(GL_VERSION);
	char* GL_vendor = (char*)glGetString(GL_VENDOR);
	char* GL_renderer = (char*)glGetString(GL_RENDERER);

	printf("Version: %s\n", GL_version);
	printf("Vendor: %s\n", GL_vendor);
	printf("Renderer: %s\n", GL_renderer);
}

void ClearColor(float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
	graphics_swap_buffers();
}

int main(int argc, char** argv) {
	graphics_setup_window(0, 0, 1920, 1080);
	InitGraphicsState();
	ClearColor(0, 0.5, 1, 1);
	usleep(1000000);
	ClearColor(0, 1, 1, 1);
	usleep(1000000);
	ClearColor(1, 0.5, 1, 1);
	usleep(1000000);
	ClearColor(1, 1, 1, 1);
	usleep(1000000);	
	graphics_close_window();
}
