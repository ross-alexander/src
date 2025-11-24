LDLIBS_SCALE	= `pkg-config --libs gdk-3.0 gegl-0.4`
CFLAGS_SCALE	= `pkg-config --cflags gdk-3.0 gegl-0.4`


ALL: scale gdkscale pixbuf2cairo draw

pixbuf2cairo: pixbuf2cairo.c
	$(CC) -std=c17 -Wall `pkg-config --cflags --libs gdk-pixbuf-2.0 cairo babl-0.1` $< -o $@

scale	: scale.o
	$(CXX) $(LDFLAGS) $^ -rdynamic $(LDLIBS_SCALE) -o $@

scale.o : scale.cc
	$(CXX) $(CXXFLAGS) $(CFLAGS_SCALE)  -c $< -o $@

gdkscale	: gdkscale.o
	$(CC) $(CFLAGS) $(LDLIBS_SCALE) $< -o $@

gdkscale.o : gdkscale.c
	$(CC) $(CFLAGS) $(CFLAGS_SCALE)  -c $< -o $@

draw	: draw.o
	$(CC) $(CFLAGS) $< `pkg-config --libs gtk+-3.0` -o $@

draw.o: draw_in_a_GdkPixbuf_with_cairo_and_pango.c
	$(CC) $(CFLAGS) `pkg-config --cflags gtk+-3.0` -c $^ -o $@

clean:
	rm -f $(ALL) *.o *~
