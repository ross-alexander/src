LDLIBS_SCALE	= `pkg-config --libs gdk-3.0 gegl-0.4`
CFLAGS_SCALE	= `pkg-config --cflags gdk-3.0 gegl-0.4`


ALL: scale gdkscale

scale	: scale.o
	$(CXX) $(LDFLAGS) $^ -rdynamic $(LDLIBS_SCALE) -o $@

scale.o : scale.cc
	$(CXX) $(CXXFLAGS) $(CFLAGS_SCALE)  -c $< -o $@

gdkscale	: gdkscale.o
	$(CC) $(CFLAGS) $(LDLIBS_SCALE) $< -o $@

gdkscale.o : gdkscale.c
	$(CC) $(CFLAGS) $(CFLAGS_SCALE)  -c $< -o $@

