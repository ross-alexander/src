CFLAGS 		= -std=gnu23 -Wall
CPPFLAGS	= $(shell pkg-config --cflags glycin-2 cairo babl-0.1)
LDLIBS		= $(shell pkg-config --libs glycin-2 cairo babl-0.1)

glycin-load: glycin-load.o

clean:
	rm -f *.o *~ glycin-load
