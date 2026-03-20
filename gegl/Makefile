CFLAGS	= $(shell pkg-config --cflags gegl-0.4)
LDLIBS	= $(shell pkg-config --libs gegl-0.4)

gegl-test-load: gegl-test-load.o

clean:
	rm -f *.o *~ gegl-test-load
