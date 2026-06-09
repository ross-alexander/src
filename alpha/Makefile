CC	= gcc
CFLAGS	= -std=c23 $(shell pkg-config --cflags babl-0.1 gegl-0.4)
LDLIBS	= $(shell pkg-config --libs babl-0.1 gegl-0.4)

all: alpha

alpha: alpha.o

clean:
	rm -f *~ *.o alpha
