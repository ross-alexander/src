CC		= gcc -std=c11 -g -D_DEFAULT_SOURCE

GLXCFLAGS	= `pkg-config --cflags x11`
GLXLIBS		= `pkg-config --libs glew x11` -lGL -lGLU -ldl

CPPFLAGS	= $(GLUTCFLAGS) -D_GNU_SOURCE `pkg-config --cflags glproto gts`
CFLAGS		= -Wall -fPIC
LDFLAGS		= `pkg-config --libs-only-L x11`
LDLIBS		=  $(GLUTLIBS) -lm `pkg-config --libs-only-l x11` -lglut -L/usr/X11/lib -lX11 -lGLU -lGL -lXi -lXmu

GTSLIBS		= `pkg-config --libs gts`

XCBLDLIBS	= `pkg-config --libs xcb-glx x11-xcb` -lGL -lGLU

ALL		= plane honeycomb test glx xcb mygl.so mygl3.so tri3.so tmath glxgears ex ex2 ex3 ex4

.suffix: .so

%.so	: %.o
	$(CC) -shared $^ -lm -o $@


all: $(ALL)

honeycomb: honeycomb.o
	$(CC) $^ $(GTSLIBS) $(GLXLIBS) -lm -lglut -o $@

test: test.o
	$(CC) $^ $(GTSLIBS) $(GLXLIBS) -lm -lglut -o $@

ex	: ex.o
	$(CC) $(XCBLDLIBS) $^ -o $@

ex2	: ex2.o
	$(CC) $(XCBLDLIBS) $^ -o $@

ex3	: ex3.o
	$(CC) $^ -lxcb -lGL -lEGL -o $@

ex4	: ex4.o
	$(CC) $^ -lxcb -lGL -lX11 -lX11-xcb -o $@


glx:	glx.o
	$(CC) $^ $(GLXLIBS) -o $@

mygl.so	: mygl.o

mygl3.so : mymath.o mygl3.o

tri3.so : mymath.o tri3.o

glx.o	: glx.c
	$(CC) -c $(GLXCFLAGS) $^ -o $@

xcb	: mygl.o xcb.o
	$(CC) $(XCBLDLIBS) $^ -o $@

tmath	: mymath.o tmath.o

clean	:
	rm -r *.o *~ $(ALL)
