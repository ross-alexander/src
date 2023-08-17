GSTREAMER = gstreamer-1.0 gstreamer-video-1.0 xcb

CFLAGS	= -g -std=c99 $(shell pkg-config --cflags $(GSTREAMER) xcb-aux xcb-atom xcb-xfixes xcb-randr)
LDLIBS	= $(shell pkg-config --libs $(GSTREAMER) x11 xcb-aux xcb-atom xcb-xfixes xcb-randr)

all 	: vorbis x264 decodebin randr metadata xcbrandr example3 fullscreen typefind tags

movewin	: movewin.o

x264	: xcb.o x264.o

decodebin: xcb.o decodebin.o

tags : tags.c
	gcc -o tags tags.c `pkg-config --cflags --libs gstreamer-1.0`
