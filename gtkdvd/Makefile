LDLIBS	= ($shell pkg-config --libs dvdread lua-5.5 gtk+-3.0 gthread-2.0) # -lreadline -lcurses
CFLAGS	= -std=c17 -fPIC -D_DEFAULT_SOURCE -D_FILE_OFFSET_BITS=64 $(shell pkg-config --cflags dvdread glib-2.0 lua-5.5)

# ALL	= gtkdvd unlock dvd.so foo.so
ALL	= unlock dvd.so

default	: $(ALL) 

unlock	: unlock.o
	$(CC) $^ -o $@

gtkdvd	: gtk.o cli.o dvd.o main.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

dvd.so	: dvd.o dvd_wrap.o
	$(CC) -shared $^  $(shell pkg-config --libs dvdread glib-2.0 lua-5.5) -o $@

dvd_wrap.o	: dvd_wrap.c
	$(CC) -c $(CFLAGS) $< -o $@

dvd_wrap.c	: dvd.swg
	swig -lua dvd.swg

clean:
	rm -f *~ *.o dvd_wrap.c $(ALL)
