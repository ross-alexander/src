LDLIBS_SCALE	= $(shell pkg-config --libs gtk+-3.0 gdk-3.0 gegl-0.4)
CFLAGS		= $(shell pkg-config --cflags gtk+-3.0 gdk-3.0 gegl-0.4)
CXX		= g++ -std=c++20

ALL: scale gdkscale pixbuf2cairo draw

pixbuf2cairo: pixbuf2cairo.c
	$(CC) -std=c17 -Wall `pkg-config --cflags --libs gdk-pixbuf-2.0 cairo babl-0.1` $< -o $@

scale	: scale.o
	$(CXX) $(LDFLAGS) $^ -rdynamic $(LDLIBS_SCALE) -o $@

scale.o : scale.cc
	$(CXX) $(CXXFLAGS) $(CFLAGS)  -c $< -o $@

gdkscale : gdkscale.o
	$(CC) $(CFLAGS) $(LDLIBS_SCALE) $< -o $@

gdkscale.o : gdkscale.c
	$(CC) $(CFLAGS) $(CFLAGS_SCALE)  -c $< -o $@

draw	: pixbuf_cairo_pango.o
	$(CC) $(CFLAGS) $< `pkg-config --libs gtk+-3.0` -o $@

pixbuf_cairo_pango.o: pixbuf_cairo_pango.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f $(ALL) *.o *~
