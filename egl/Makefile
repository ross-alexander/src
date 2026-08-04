CPPFLAGS = `pkg-config --cflags libdrm`
CXXFLAGS = -std=c++20
LDLIBS	= -lEGL
DRMLIBS	= -lEGL -ldrm -lGL -lGLU

ALL = eglinfo egl-gbm stream-01 egl-example


all	: $(ALL)

stream-01: stream-01.o
	$(CC) $^ $(DRMLIBS) -o $@

egl-example: egl-example.o
	$(CC) $^ -lepoxy -ldrm -lEGL -lGLU -lOpenGL -o $@

egl-example.o: egl-example.c
	$(CC) -c -I/usr/include/drm -DEGL_NO_X11=1 $< -o $@

clean:
	rm -f $(ALL) *.o *~
