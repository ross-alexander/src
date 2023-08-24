CFLAGS		= -Wall -std=c99 -g -D_GNU_SOURCE -fPIC -I.
CXXFLAGS	= -Wall -std=c++14 -g -D_GNU_SOURCE -fPIC -I.
LDFLAGS	=

CFLAGS_GDK	= `pkg-config --cflags gdk-3.0`
CFLAGS_CAIRO	= `pkg-config --cflags gdk-3.0 cairo`
CFLAGS_GTK2	= `pkg-config --cflags gtk+-2.0`
CFLAGS_GTK3	= `pkg-config --cflags gtk+-3.0`
CFLAGS_GTK4	= `pkg-config --cflags gtk4`
CFLAGS_GLX	= `pkg-config --cflags x11`
CFLAGS_XAW	= `pkg-config --cflags cairo-xlib xaw7`
CFLAGS_XM	= `motif-config --cflags`
CFLAGS_XLIB	= `pkg-config --cflags cairo-xlib x11`
CFLAGS_XCB	= `pkg-config --cflags cairo-xcb xcb`
CFLAGS_XTK	= `pkg-config --cflags x11`
CFLAGS_GEGL	= `pkg-config --cflags gegl-0.4`

LDLIBS_GDK	= `pkg-config --libs gdk-3.0`
LDLIBS_GLX	= `pkg-config --libs x11` -lGLU
LDLIBS_XAW	= `pkg-config --libs xaw7`
LDLIBS_XCB	= `pkg-config --libs xcb-aux xcb-atom xcb`
LDLIBS_XM	= `motif-config --libs`
LDLIBS_GTK2	= `pkg-config --libs gtk+-2.0`
LDLIBS_GTK3	= `pkg-config --libs gtk+-3.0`
LDLIBS_GTK4	= `pkg-config --libs gtk4`
LDLIBS_XTK	= `pkg-config --libs gegl-0.4 cairo`

RENDER = render/gegl.so render/gdk.so render/cairo.so 
DISPLAY = display/xcb.so display/xlib.so display/xaw.so display/gtk2.so display/gtk3.so display/gtk4.so

ALL	= xtk $(RENDER) $(DISPLAY)


.suffix	: .so

%.so	: %.o
	$(CC) -shared $^ -o $@

all	: $(ALL)

xtk	: draw.o xtk.o
	$(CC) $(CFLAGS) $^ -rdynamic $(LDLIBS_XTK) -ldl -o $@

xtk.o	: xtk.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_XTK) -c $< -o $@

draw.o	: draw.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_CAIRO) -c $< -o $@

# Display libraries

display/gtk2.so	: gtk2.o
	$(CC) $(LDFLAGS) $^ -shared $(LDLIBS_GTK2) -o $@

gtk2.o	: gtk2.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_GTK2) -c $< -o $@

display/gtk3.so	: gtk3.o
	$(CC) $(LDFLAGS) $^ -shared $(LDLIBS_GTK3) -o $@

gtk3.o	: gtk3.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_GTK3) -c $< -o $@

display/gtk4.so	: gtk4.o
	$(CC) $(LDFLAGS) $^ -shared $(LDLIBS_GTK4) -o $@

gtk4.o	: gtk4.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_GTK4) -c $< -o $@

display/xaw.so	: visual.o xaw.o
	$(CC) $(LDFLAGS) $^ -shared $(LDLIBS_XAW) -o $@

xaw.o	: xaw.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_XAW) -c $< -o $@

display/xlib.so	: visual.o xlib.o
	$(CC) $(LDFLAGS) $^ -shared $(LDLIBS_XLIB) -o $@

xlib.o	: xlib.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_XLIB) -c $< -o $@

display/xcb.so	: xcb.o
	$(CC) $(LDFLAGS) $^ -shared $(LDLIBS_XCB) -o $@

xcb.o	: xcb.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_XCB) -c $< -o $@

xm.so	: visual.o xm.o
	$(CC) $(LDFLAGS) $^ -shared $(LDLIBS_XM) -o $@

xm.o	: xm.c
	$(CC) $(CFLAGS) $(CFLAGS_XM) -c $< -o $@

display/glx.so	: pbinfo.o pbutil.o glx.o
	$(CC) $(LDFLAGS) $^ -shared $(LDLIBS_GLX) -o $@

glx.o	: glx.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_GLX) -c $< -o $@

visual.o	: visual.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_XLIB) -c $< -o $@

# Render libraries

cairo.o: cairo.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_CAIRO) -c $< -o $@

render/cairo.so : cairo.o
	$(CC) $(LDFLAGS) $^ -shared $(LDLIBS_XTK) -o $@

gdk.o : gdk.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_GDK) -c $< -o $@

render/gdk.so : gdk.o
	$(CC) $(LDFLAGS) $^ -shared $(LDLIBS_GDK) -o $@

gegl.o : gegl.c common.h
	$(CC) $(CFLAGS) $(CFLAGS_GEGL) -c $< -o $@

render/gegl.so : gegl.o
	$(CC) $(LDFLAGS) $^ -shared $(LDLIBS_GEGL) -o $@


clean:
	rm -f *.o *~ $(ALL)

