#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

#ifdef LIBEPOXY
#include <epoxy/egl.h>
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

PFNEGLQUERYDEVICESEXTPROC pEglQueryDevicesEXT = NULL;
PFNEGLQUERYDEVICESTRINGEXTPROC pEglQueryDeviceStringEXT = NULL;
PFNEGLGETPLATFORMDISPLAYEXTPROC pEglGetPlatformDisplayEXT = NULL;
PFNEGLGETOUTPUTLAYERSEXTPROC pEglGetOutputLayersEXT = NULL;
PFNEGLCREATESTREAMKHRPROC pEglCreateStreamKHR = NULL;
PFNEGLSTREAMCONSUMEROUTPUTEXTPROC pEglStreamConsumerOutputEXT = NULL;
PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC pEglCreateStreamProducerSurfaceKHR = NULL;

#define ARRAY_LEN(_arr) (sizeof(_arr) / sizeof(_arr[0]))

struct Config {
    uint32_t connectorID;
    uint32_t crtcID;
    int crtcIndex;
    uint32_t planeID;
    drmModeModeInfo mode;
    uint16_t width;
    uint16_t height;
};

struct PropertyIDs {

    struct {
        uint32_t mode_id;
        uint32_t active;
    } crtc;

    struct {
        uint32_t src_x;
        uint32_t src_y;
        uint32_t src_w;
        uint32_t src_h;
        uint32_t crtc_x;
        uint32_t crtc_y;
        uint32_t crtc_w;
        uint32_t crtc_h;
        uint32_t fb_id;
        uint32_t crtc_id;
    } plane;

    struct {
        uint32_t crtc_id;
    } connector;
};

struct PropertyIDAddresses {
    const char *name;
    uint32_t *ptr;
};


void Fatal(const char *format, ...)
{
    va_list ap;

    fprintf(stderr, "ERROR: ");

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    exit(1);
}

EGLBoolean ExtensionIsSupported(const char *extensionString,
                                const char *extension)
{
    const char *endOfExtensionString;
    const char *currentExtension = extensionString;
    size_t extensionLength;

    if ((extensionString == NULL) || (extension == NULL)) {
        return EGL_FALSE;
    }

    extensionLength = strlen(extension);

    endOfExtensionString = extensionString + strlen(extensionString);

    while (currentExtension < endOfExtensionString) {
        const size_t currentExtensionLength = strcspn(currentExtension, " ");
        if ((extensionLength == currentExtensionLength) &&
            (strncmp(extension, currentExtension,
                     extensionLength) == 0)) {
            return EGL_TRUE;
        }
        currentExtension += (currentExtensionLength + 1);
    }
    return EGL_FALSE;
}

int GetDrmFd(EGLDeviceEXT device)
{
    const char *deviceExtensionString = pEglQueryDeviceStringEXT(device, EGL_EXTENSIONS);

    const char *drmDeviceFile;
    int fd;

    if (!ExtensionIsSupported(deviceExtensionString, "EGL_EXT_device_drm"))
      {
        Fatal("EGL_EXT_device_drm extension not found.\n");
      }

    drmDeviceFile = pEglQueryDeviceStringEXT(device, EGL_DRM_DEVICE_FILE_EXT);

    if (drmDeviceFile == NULL)
      {
        Fatal("No DRM device file found for EGL device.\n");
      }

    fd = open(drmDeviceFile, O_RDWR, 0);

    if (fd < 0) {
        Fatal("Unable to open DRM device file.\n");
    }
    return fd;
}


EGLDeviceEXT GetEglDevice(void)
{
    EGLint numDevices, i;
    EGLDeviceEXT *devices = NULL;
    EGLDeviceEXT device = EGL_NO_DEVICE_EXT;
    EGLBoolean ret;

    const char *clientExtensionString =
        eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);

    if (!ExtensionIsSupported(clientExtensionString,
                              "EGL_EXT_device_base") &&
        (!ExtensionIsSupported(clientExtensionString,
                               "EGL_EXT_device_enumeration") ||
         !ExtensionIsSupported(clientExtensionString,
                               "EGL_EXT_device_query"))) {
        Fatal("EGL_EXT_device base extensions not found.\n");
    }

    /* Query how many devices are present. */
    ret = pEglQueryDevicesEXT(0, NULL, &numDevices);

    if (!ret) {
        Fatal("Failed to query EGL devices.\n");
    }

    if (numDevices < 1) {
        Fatal("No EGL devices found.\n");
    }

    /* Allocate memory to store that many EGLDeviceEXTs. */
    devices = calloc(numDevices, sizeof(EGLDeviceEXT));

    if (devices == NULL) {
        Fatal("Memory allocation failure.\n");
    }

    /* Query the EGLDeviceEXTs. */
    ret = pEglQueryDevicesEXT(numDevices, devices, &numDevices);

    if (!ret) {
        Fatal("Failed to query EGL devices.\n");
    }

    /*
     * Select which EGLDeviceEXT to use.
     *
     * The EGL_EXT_device_query extension defines the functions:
     *
     *   eglQueryDeviceAttribEXT()
     *   eglQueryDeviceStringEXT()
     *
     * as ways to generically query properties of EGLDeviceEXTs, and
     * separate EGL extensions define EGLDeviceEXT attributes that can
     * be queried through those functions.  E.g.,
     *
     * - EGL_NV_device_cuda lets you query the CUDA device ID
     *   (EGL_CUDA_DEVICE_NV of an EGLDeviceEXT.
     *
     * - EGL_EXT_device_drm lets you query the DRM device file
     *   (EGL_DRM_DEVICE_FILE_EXT) of an EGLDeviceEXT.
     *
     * Future extensions could define other EGLDeviceEXT attributes
     * such as PCI BusID.
     *
     * For now, just choose the first device that supports EGL_EXT_device_drm.
     */

    for (i = 0; i < numDevices; i++) {

        const char *deviceExtensionString =
            pEglQueryDeviceStringEXT(devices[i], EGL_EXTENSIONS);

        if (ExtensionIsSupported(deviceExtensionString, "EGL_EXT_device_drm")) {
            device = devices[i];
            break;
        }
    }

    free(devices);

    if (device == EGL_NO_DEVICE_EXT) {
        Fatal("No EGL_EXT_device_drm-capable EGL device found.\n");
    }

    return device;
}


/*
 * Search for the specified property on the given object, and return
 * its value.
 */
static uint64_t GetPropertyValue(
    int drmFd,
    uint32_t objectID,
    uint32_t objectType,
    const char *propName)
{
    uint32_t i;
    int found = 0;
    uint64_t value = 0;
    drmModeObjectPropertiesPtr pModeObjectProperties =
        drmModeObjectGetProperties(drmFd, objectID, objectType);

    for (i = 0; i < pModeObjectProperties->count_props; i++) {

        drmModePropertyPtr pProperty =
            drmModeGetProperty(drmFd, pModeObjectProperties->props[i]);

        if (pProperty == NULL) {
            Fatal("Unable to query property.\n");
        }

        if (strcmp(propName, pProperty->name) == 0) {
            value = pModeObjectProperties->prop_values[i];
            found = 1;
        }

        drmModeFreeProperty(pProperty);

        if (found) {
            break;
        }
    }

    drmModeFreeObjectProperties(pModeObjectProperties);

    if (!found) {
        Fatal("Unable to find value for property \'%s\'.\n", propName);
    }

    return value;
}


/*
 * Query the properties for the specified object, and populate the IDs
 * in the given table.
 */
static void AssignPropertyIDsOneType(int drmFd,
                                     uint32_t objectID,
                                     uint32_t objectType,
                                     struct PropertyIDAddresses *table,
                                     size_t tableLen)
{
    uint32_t i;
    drmModeObjectPropertiesPtr pModeObjectProperties =
        drmModeObjectGetProperties(drmFd, objectID, objectType);

    if (pModeObjectProperties == NULL) {
        Fatal("Unable to query mode object properties.\n");
    }

    for (i = 0; i < pModeObjectProperties->count_props; i++) {

        uint32_t j;
        drmModePropertyPtr pProperty =
            drmModeGetProperty(drmFd, pModeObjectProperties->props[i]);

        if (pProperty == NULL) {
            Fatal("Unable to query property.\n");
        }

        for (j = 0; j < tableLen; j++) {
            if (strcmp(table[j].name, pProperty->name) == 0) {
                *(table[j].ptr) = pProperty->prop_id;
                break;
            }
        }

        drmModeFreeProperty(pProperty);
    }

    drmModeFreeObjectProperties(pModeObjectProperties);

    for (i = 0; i < tableLen; i++) {
        if (*(table[i].ptr) == 0) {
            Fatal("Unable to find property ID for \'%s\'.\n", table[i].name);
        }
    }
}


/*
 * Find the property IDs for the CRTC, plane, and connector in the
 * Config.
 */
static void AssignPropertyIDs(int drmFd,
                              const struct Config *pConfig,
                              struct PropertyIDs *pPropertyIDs)
{
    struct PropertyIDAddresses crtcTable[] = {
        { "MODE_ID", &pPropertyIDs->crtc.mode_id      },
        { "ACTIVE",  &pPropertyIDs->crtc.active       },
    };

    struct PropertyIDAddresses planeTable[] = {
        { "SRC_X",   &pPropertyIDs->plane.src_x       },
        { "SRC_Y",   &pPropertyIDs->plane.src_y       },
        { "SRC_W",   &pPropertyIDs->plane.src_w       },
        { "SRC_H",   &pPropertyIDs->plane.src_h       },
        { "CRTC_X",  &pPropertyIDs->plane.crtc_x      },
        { "CRTC_Y",  &pPropertyIDs->plane.crtc_y      },
        { "CRTC_W",  &pPropertyIDs->plane.crtc_w      },
        { "CRTC_H",  &pPropertyIDs->plane.crtc_h      },
        { "FB_ID",   &pPropertyIDs->plane.fb_id       },
        { "CRTC_ID", &pPropertyIDs->plane.crtc_id     },
    };

    struct PropertyIDAddresses connectorTable[] = {
        { "CRTC_ID", &pPropertyIDs->connector.crtc_id },
    };

    AssignPropertyIDsOneType(drmFd, pConfig->crtcID,
                             DRM_MODE_OBJECT_CRTC,
                             crtcTable, ARRAY_LEN(crtcTable));
    AssignPropertyIDsOneType(drmFd, pConfig->planeID,
                             DRM_MODE_OBJECT_PLANE,
                             planeTable, ARRAY_LEN(planeTable));
    AssignPropertyIDsOneType(drmFd, pConfig->connectorID,
                             DRM_MODE_OBJECT_CONNECTOR,
                             connectorTable, ARRAY_LEN(connectorTable));
}


/*
 * A KMS atomic request is made by "adding properties" to a
 * drmModeAtomicReqPtr object.
 *
 * Find the property IDs that we need to describe the request, then
 * add the properties to the request.
 */
static void AssignAtomicRequest(int drmFd,
                                drmModeAtomicReqPtr pAtomic,
                                const struct Config *pConfig,
                                uint32_t modeID, uint32_t fb)
{
    struct PropertyIDs propertyIDs = { 0 };

    AssignPropertyIDs(drmFd, pConfig, &propertyIDs);


    /* Specify the mode to use on the CRTC, and make the CRTC active. */

    drmModeAtomicAddProperty(pAtomic, pConfig->crtcID,
                             propertyIDs.crtc.mode_id, modeID);
    drmModeAtomicAddProperty(pAtomic, pConfig->crtcID,
                             propertyIDs.crtc.active, 1);

    /* Tell the connector to receive pixels from the CRTC. */

    drmModeAtomicAddProperty(pAtomic, pConfig->connectorID,
                             propertyIDs.connector.crtc_id, pConfig->crtcID);

    /*
     * Specify the region of source surface to display (i.e., the
     * "ViewPortIn").  Note these values are in 16.16 format, so shift
     * up by 16.
     */

    drmModeAtomicAddProperty(pAtomic, pConfig->planeID,
                             propertyIDs.plane.src_x, 0);
    drmModeAtomicAddProperty(pAtomic, pConfig->planeID,
                             propertyIDs.plane.src_y, 0);
    drmModeAtomicAddProperty(pAtomic, pConfig->planeID,
                             propertyIDs.plane.src_w, pConfig->width << 16);
    drmModeAtomicAddProperty(pAtomic, pConfig->planeID,
                             propertyIDs.plane.src_h, pConfig->height << 16);

    /*
     * Specify the region within the mode where the image should be
     * displayed (i.e., the "ViewPortOut").
     */

    drmModeAtomicAddProperty(pAtomic, pConfig->planeID,
                             propertyIDs.plane.crtc_x, 0);
    drmModeAtomicAddProperty(pAtomic, pConfig->planeID,
                             propertyIDs.plane.crtc_y, 0);
    drmModeAtomicAddProperty(pAtomic, pConfig->planeID,
                             propertyIDs.plane.crtc_w, pConfig->width);
    drmModeAtomicAddProperty(pAtomic, pConfig->planeID,
                             propertyIDs.plane.crtc_h, pConfig->height);

    /*
     * Specify the surface to display in the plane, and connect the
     * plane to the CRTC.
     *
     * XXX for EGLStreams purposes, it would be nice to have the
     * option of not specifying a surface at this point, as well as to
     * be able to have the KMS atomic modeset consume a frame from an
     * EGLStream.
     */

    drmModeAtomicAddProperty(pAtomic, pConfig->planeID,
                             propertyIDs.plane.fb_id, fb);
    drmModeAtomicAddProperty(pAtomic, pConfig->planeID,
                             propertyIDs.plane.crtc_id, pConfig->crtcID);
}




static void PickPlane(int drmFd, struct Config *pConfig)
{
    drmModePlaneResPtr pPlaneRes = drmModeGetPlaneResources(drmFd);
    uint32_t i;

    if (pPlaneRes == NULL) {
        Fatal("Unable to query DRM-KMS plane resources\n");
    }

    for (i = 0; i < pPlaneRes->count_planes; i++) {
        drmModePlanePtr pPlane = drmModeGetPlane(drmFd, pPlaneRes->planes[i]);
        uint32_t crtcs;
        uint64_t type;

        if (pPlane == NULL) {
            Fatal("Unable to query DRM-KMS plane %d\n", i);
        }

        crtcs = pPlane->possible_crtcs;

        drmModeFreePlane(pPlane);

        if ((crtcs & (1 << pConfig->crtcIndex)) == 0) {
            continue;
        }

        type = GetPropertyValue(drmFd, pPlaneRes->planes[i],
                                DRM_MODE_OBJECT_PLANE, "type");

        if (type == DRM_PLANE_TYPE_PRIMARY) {
            pConfig->planeID = pPlaneRes->planes[i];
            break;
        }
    }

    drmModeFreePlaneResources(pPlaneRes);

    if (pConfig->planeID == 0) {
        Fatal("Could not find a suitable plane.\n");
    }
}

/*
 * Create a blank DRM fb object.
 */
static uint32_t CreateFb(int drmFd, const struct Config *pConfig)
{
    struct drm_mode_create_dumb createRequest = { 0 };
    struct drm_mode_map_dumb mapRequest = { 0 };
    uint8_t *map;
    uint32_t fb = 0;
    int ret;

    createRequest.width = pConfig->width;
    createRequest.height = pConfig->height;
    createRequest.bpp = 32;

    ret = drmIoctl(drmFd, DRM_IOCTL_MODE_CREATE_DUMB, &createRequest);
    if (ret < 0) {
        Fatal("Unable to create dumb buffer.\n");
    }

    ret = drmModeAddFB(drmFd, pConfig->width, pConfig->height, 24, 32,
                       createRequest.pitch, createRequest.handle, &fb);
    if (ret) {
        Fatal("Unable to add fb.\n");
    }

    mapRequest.handle = createRequest.handle;

    ret = drmIoctl(drmFd, DRM_IOCTL_MODE_MAP_DUMB, &mapRequest);
    if (ret) {
        Fatal("Unable to map dumb buffer.\n");
    }

    map = mmap(0, createRequest.size, PROT_READ | PROT_WRITE, MAP_SHARED,
               drmFd, mapRequest.offset);
    if (map == MAP_FAILED) {
        Fatal("Failed to mmap(2) fb.\n");
    }

    memset(map, 0, createRequest.size);

    return fb;
}




/*
 * Create an ID for the mode in the specified config.
 */
static uint32_t CreateModeID(int drmFd, const struct Config *pConfig)
{
  uint32_t modeID = 0;
  int ret = drmModeCreatePropertyBlob(drmFd, &pConfig->mode, sizeof(pConfig->mode), &modeID);
  if (ret != 0)
    {
      Fatal("Failed to create mode property.\n");
    }
  return modeID;
}


/*
 * Pick the first connected connector we find with usable modes and
 * CRTC.
 */
static void PickConnector(int drmFd,
                          drmModeResPtr pModeRes,
                          struct Config *pConfig)
{
    int i, j;

    for (i = 0; i < pModeRes->count_connectors; i++) {

        drmModeConnectorPtr pConnector =
            drmModeGetConnector(drmFd, pModeRes->connectors[i]);

        if (pConnector == NULL) {
            Fatal("Unable to query DRM-KMS information for "
                  "connector index %d\n", i);
        }

        if ((pConnector->connection == DRM_MODE_CONNECTED) &&
            (pConnector->count_modes > 0) &&
            (pConnector->count_encoders > 0)) {

            drmModeEncoderPtr pEncoder =
                drmModeGetEncoder(drmFd, pConnector->encoders[0]);

            if (pEncoder == NULL) {
                Fatal("Unable to query DRM-KMS information for"
                      "encoder 0x%08x\n", pConnector->encoders[0]);
            }

            pConfig->connectorID = pModeRes->connectors[i];
            pConfig->mode = pConnector->modes[0];

            for (j = 0; j < pModeRes->count_crtcs; j++) {

                if ((pEncoder->possible_crtcs & (1 << j)) == 0) {
                    continue;
                }

                pConfig->crtcID = pModeRes->crtcs[j];
                pConfig->crtcIndex = j;
                break;
            }

            if (pConfig->crtcID == 0) {
                Fatal("Unable to select a suitable CRTC.\n");
            }

            drmModeFreeEncoder(pEncoder);
        }

        drmModeFreeConnector(pConnector);
    }

    if (pConfig->connectorID == 0) {
        Fatal("Could not find a suitable connector.\n");
    }

    if (pConfig->crtcID == 0) {
        Fatal("Could not find a suitable CRTC.\n");
    }
}

/*
 * Pick a connector, CRTC, and plane to use for the modeset.
 */
static void PickConfig(int drmFd, struct Config *pConfig)
{
    drmModeResPtr pModeRes;
    int ret;

    ret = drmSetClientCap(drmFd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    if (ret != 0) {
        Fatal("DRM_CLIENT_CAP_UNIVERSAL_PLANES not available.\n");
    }

    ret = drmSetClientCap(drmFd, DRM_CLIENT_CAP_ATOMIC, 1);

    if (ret != 0) {
        Fatal("DRM_CLIENT_CAP_ATOMIC not available.\n");
    }

    pModeRes = drmModeGetResources(drmFd);

    if (pModeRes == NULL) {
        Fatal("Unable to query DRM-KMS resources.\n");
    }

    PickConnector(drmFd, pModeRes, pConfig);

    PickPlane(drmFd, pConfig);

    drmModeFreeResources(pModeRes);

    pConfig->width = pConfig->mode.hdisplay;
    pConfig->height = pConfig->mode.vdisplay;
}


void SetMode(int drmFd, uint32_t *pPlaneID, int *pWidth, int *pHeight)
{
    struct Config config = { 0 };
    drmModeAtomicReqPtr pAtomic;
    uint32_t modeID, fb;
    int ret;
    const uint32_t flags = DRM_MODE_ATOMIC_ALLOW_MODESET;

    PickConfig(drmFd, &config);

    modeID = CreateModeID(drmFd, &config);
    fb = CreateFb(drmFd, &config);

    pAtomic = drmModeAtomicAlloc();

    AssignAtomicRequest(drmFd, pAtomic, &config, modeID, fb);

    ret = drmModeAtomicCommit(drmFd, pAtomic, flags, NULL /* user_data */);

    drmModeAtomicFree(pAtomic);

    if (ret != 0) {
        Fatal("Failed to set mode.\n");
    }

    *pPlaneID = config.planeID;
    *pWidth = config.width;
    *pHeight = config.height;
}

/*
 * Create an EGLDisplay from the given EGL device.
 */
EGLDisplay GetEglDisplay(EGLDeviceEXT device, int drmFd)
{
  EGLDisplay eglDpy;

  const char *clientExtensionString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
  const char *deviceExtensionString = pEglQueryDeviceStringEXT(device, EGL_EXTENSIONS);

    /*
     * Provide the DRM fd when creating the EGLDisplay, so that the
     * EGL implementation can make any necessary DRM calls using the
     * same fd as the application.
     */
    EGLint attribs[] = {
        EGL_DRM_MASTER_FD_EXT,
        drmFd,
        EGL_NONE
    };

    /*
     * eglGetPlatformDisplayEXT requires EGL client extension
     * EGL_EXT_platform_base.
     */
    if (!ExtensionIsSupported(clientExtensionString, "EGL_EXT_platform_base"))
      {
        Fatal("EGL_EXT_platform_base not found.\n");
      }

    /*
     * EGL_EXT_platform_device is required to pass
     * EGL_PLATFORM_DEVICE_EXT to eglGetPlatformDisplayEXT().
     */

    if (!ExtensionIsSupported(clientExtensionString, "EGL_EXT_platform_device"))
      {
        Fatal("EGL_EXT_platform_device not found.\n");
      }

    /*
     * Providing a DRM fd during EGLDisplay creation requires
     * EGL_EXT_device_drm.
     */
    if (!ExtensionIsSupported(deviceExtensionString, "EGL_EXT_device_drm"))
      {
        Fatal("EGL_EXT_device_drm not found.\n");
      }

    /* Get an EGLDisplay from the EGLDeviceEXT. */
    eglDpy = pEglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, (void*)device, attribs);

    if (eglDpy == EGL_NO_DISPLAY)
      {
        Fatal("Failed to get EGLDisplay from EGLDevice.");
      }

    if (!eglInitialize(eglDpy, NULL, NULL))
      {
        Fatal("Failed to initialize EGLDisplay.");
      }
    return eglDpy;
}


/* ----------------------------------------------------------------------
 *
 * Set up EGL to present to a DRM KMS plane through an EGLStream.
 *
 ---------------------------------------------------------------------- */

EGLSurface SetUpEgl(EGLDisplay eglDpy, uint32_t planeID, int width, int height)
{
    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_STREAM_BIT_KHR,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 1,
        EGL_NONE,
    };

    EGLint contextAttribs[] = { EGL_NONE };

    EGLAttrib layerAttribs[] = {
        EGL_DRM_PLANE_EXT,
        planeID,
        EGL_NONE,
    };

    EGLint streamAttribs[] = { EGL_NONE };

    EGLint surfaceAttribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE
    };

    EGLConfig eglConfig;
    EGLContext eglContext;
    EGLint n = 0;
    EGLBoolean ret;
    EGLOutputLayerEXT eglLayer;
    EGLStreamKHR eglStream;
    EGLSurface eglSurface;

    const char *extensionString = eglQueryString(eglDpy, EGL_EXTENSIONS);

    /*
     * EGL_EXT_output_base and EGL_EXT_output_drm are needed to find
     * the EGLOutputLayer for the DRM KMS plane.
     */

    if (!ExtensionIsSupported(extensionString, "EGL_EXT_output_base"))
      {
        Fatal("EGL_EXT_output_base not found.\n");
      }

    if (!ExtensionIsSupported(extensionString, "EGL_EXT_output_drm"))
      {
        Fatal("EGL_EXT_output_drm not found.\n");
      }

    /*
     * EGL_KHR_stream, EGL_EXT_stream_consumer_egloutput, and
     * EGL_KHR_stream_producer_eglsurface are needed to create an
     * EGLStream connecting an EGLSurface and an EGLOutputLayer.
     */

    if (!ExtensionIsSupported(extensionString, "EGL_KHR_stream"))
      {
        Fatal("EGL_KHR_stream not found.\n");
      }
    
    if (!ExtensionIsSupported(extensionString, "EGL_EXT_stream_consumer_egloutput"))
      {
        Fatal("EGL_EXT_stream_consumer_egloutput not found.\n");
      }
    
    if (!ExtensionIsSupported(extensionString, "EGL_KHR_stream_producer_eglsurface"))
      {
        Fatal("EGL_KHR_stream_producer_eglsurface not found.\n");
      }
    
    /* Bind full OpenGL as EGL's client API. */

    eglBindAPI(EGL_OPENGL_API);

    /* Find a suitable EGL config. */

    ret = eglChooseConfig(eglDpy, configAttribs, &eglConfig, 1, &n);

    if (!ret || !n) {
        Fatal("eglChooseConfig() failed.\n");
    }

    /* Create an EGL context using the EGL config. */

    eglContext = eglCreateContext(eglDpy, eglConfig, EGL_NO_CONTEXT, contextAttribs);

    if (eglContext == NULL)
      {
        Fatal("eglCreateContext() failed.\n");
      }

    /* Find the EGLOutputLayer that corresponds to the DRM KMS plane. */

    ret = pEglGetOutputLayersEXT(eglDpy, layerAttribs, &eglLayer, 1, &n);
        
    if (!ret || !n) {
        Fatal("Unable to get EGLOutputLayer for plane 0x%08x\n", planeID);
    }

    /* Create an EGLStream. */

    eglStream = pEglCreateStreamKHR(eglDpy, streamAttribs);

    if (eglStream == EGL_NO_STREAM_KHR) {
        Fatal("Unable to create stream.\n");
    }

    /* Set the EGLOutputLayer as the consumer of the EGLStream. */

    ret = pEglStreamConsumerOutputEXT(eglDpy, eglStream, eglLayer);

    if (!ret) {
        Fatal("Unable to create EGLOutput stream consumer.\n");
    }

    /*
     * EGL_KHR_stream defines that normally stream consumers need to
     * explicitly retrieve frames from the stream.  That may be useful
     * when we attempt to better integrate
     * EGL_EXT_stream_consumer_egloutput with DRM atomic KMS requests.
     * But, EGL_EXT_stream_consumer_egloutput defines that by default:
     *
     *   On success, <layer> is bound to <stream>, <stream> is placed
     *   in the EGL_STREAM_STATE_CONNECTING_KHR state, and EGL_TRUE is
     *   returned.  Initially, no changes occur to the image displayed
     *   on <layer>. When the <stream> enters state
     *   EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR, <layer> will begin
     *   displaying frames, without further action required on the
     *   application's part, as they become available, taking into
     *   account any timestamps, swap intervals, or other limitations
     *   imposed by the stream or producer attributes.
     *
     * So, eglSwapBuffers() (to produce new frames) is sufficient for
     * the frames to be displayed.  That behavior can be altered with
     * the EGL_EXT_stream_acquire_mode extension.
     */

    /*
     * Create an EGLSurface as the producer of the EGLStream.  Once
     * the stream's producer and consumer are defined, the stream is
     * ready to use.  eglSwapBuffers() calls for the EGLSurface will
     * deliver to the stream's consumer, i.e., the DRM KMS plane
     * corresponding to the EGLOutputLayer.
     */

    eglSurface = pEglCreateStreamProducerSurfaceKHR(eglDpy, eglConfig, eglStream, surfaceAttribs);
    if (eglSurface == EGL_NO_SURFACE) {
        Fatal("Unable to create EGLSurface stream producer.\n");
    }

    /*
     * Make current to the EGLSurface, so that OpenGL rendering is
     * directed to it.
     */

    ret = eglMakeCurrent(eglDpy, eglSurface, eglSurface, eglContext);

    if (!ret) {
        Fatal("Unable to make context and surface current.\n");
    }

    return eglSurface;
}

static void *GetProcAddress(const char *functionName)
{
    void *ptr = (void *) eglGetProcAddress(functionName);

    if (ptr == NULL) {
        Fatal("eglGetProcAddress(%s) failed.\n", functionName);
    }
    return ptr;
}

void GetEglExtensionFunctionPointers(void)
{
    pEglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)
        GetProcAddress("eglQueryDevicesEXT");

    pEglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC)
        GetProcAddress("eglQueryDeviceStringEXT");

    pEglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
        GetProcAddress("eglGetPlatformDisplayEXT");

    pEglGetOutputLayersEXT = (PFNEGLGETOUTPUTLAYERSEXTPROC)
        GetProcAddress("eglGetOutputLayersEXT");

    pEglCreateStreamKHR = (PFNEGLCREATESTREAMKHRPROC)
        GetProcAddress("eglCreateStreamKHR");

    pEglStreamConsumerOutputEXT = (PFNEGLSTREAMCONSUMEROUTPUTEXTPROC)
        GetProcAddress("eglStreamConsumerOutputEXT");

    pEglCreateStreamProducerSurfaceKHR = (PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC)
        GetProcAddress("eglCreateStreamProducerSurfaceKHR");
}

int main()
{
    EGLDeviceEXT eglDevice;

#ifdef LIBEPOXY
    if (epoxy_has_egl() == 0)
      {
	Fatal("Cannot load EGL");
      }
#endif
    
    GetEglExtensionFunctionPointers();

    eglDevice = GetEglDevice();
    int drmFd = GetDrmFd(eglDevice);
    int width, height;
    uint32_t planeID = 0;
    
    SetMode(drmFd, &planeID, &width, &height);    

    EGLDisplay eglDpy = GetEglDisplay(eglDevice, drmFd);
    EGLSurface eglSurface = SetUpEgl(eglDpy, planeID, width, height);
    
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_SMOOTH);

    printf("width: %d  height: %d\n", width, height);
    
    glViewport (0, 0, (GLsizei) width, (GLsizei) height);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    if (width <= height)
      gluOrtho2D (0.0, 30.0, 0.0, 30.0 * (GLfloat) height/(GLfloat) width);
    else
      gluOrtho2D (0.0, 30.0 * (GLfloat) width/(GLfloat) height, 0.0, 30.0);
    glMatrixMode(GL_MODELVIEW);
    
    glClear (GL_COLOR_BUFFER_BIT);
    glBegin (GL_TRIANGLES);
    glColor3f (1.0, 0.0, 0.0);
    glVertex2f (5.0, 5.0);
    glColor3f (0.0, 1.0, 0.0);
    glVertex2f (25.0, 5.0);
    glColor3f (0.0, 0.0, 1.0);
    glVertex2f (5.0, 25.0);
    glEnd();
    glFlush ();
    eglSwapBuffers(eglDpy, eglSurface);
    sleep(5);
}
