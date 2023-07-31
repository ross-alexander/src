#include <stdlib.h>
#include <assert.h>

#include <fcntl.h>
#include <unistd.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <gbm.h>
#include <assert.h>


int main( void )
{
    assert( eglBindAPI( EGL_OPENGL_API ) == EGL_TRUE );

    int fd = open("/dev/dri/card0", O_RDWR);
    struct gbm_device * gbm = gbm_create_device( fd );

    assert(gbm);

    EGLDisplay dpy = eglGetDisplay( gbm );
    eglInitialize( dpy , NULL , NULL );

    assert(dpy);
    
    EGLConfig config;
    EGLint n_of_configs;
    assert( eglGetConfigs( dpy , &config , 1 , &n_of_configs ) == EGL_TRUE );

    EGLSurface srf = eglCreatePbufferSurface( dpy , config , NULL );
    assert( srf != EGL_NO_SURFACE );

    EGLContext ctx = eglCreateContext( dpy , config , EGL_NO_CONTEXT , NULL );
    assert( ctx != EGL_NO_CONTEXT );

    assert( eglMakeCurrent( dpy , srf , srf , ctx ) == EGL_TRUE );

    eglDestroySurface( dpy , srf );
    eglDestroyContext( dpy , ctx );
    eglTerminate( dpy );

    gbm_device_destroy( gbm );
    close( fd );

    return EXIT_SUCCESS;
}
