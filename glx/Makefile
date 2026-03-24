CC		= gcc -std=c23 -g -D_DEFAULT_SOURCE

GLXCFLAGS	= $(shell pkg-config --cflags x11)
GLXLIBS		= $(shell pkg-config --libs glew x11) -lGL -lGLU -ldl

CPPFLAGS	= $(GLUTCFLAGS)  $(shell pkg-config --cflags glproto gts)
CFLAGS		= -Wall -fPIC
LDFLAGS		= $(shell pkg-config --libs x11)
LDLIBS		=  $(GLUTLIBS) -lm $(shell pkg-config --libs x11) -lglut -lGLU -lGL -lXi -lXmu

GTSLIBS		= $(shell pkg-config --libs gts)

XCBLDLIBS	= $(shell pkg-config --libs xcb-glx x11-xcb) -lGL -lGLU

ALL		= plane honeycomb test glx xcb mygl.so mygl3.so tri3.so tmath glxgears ex01 ex02 ex03 ex04

.suffix: .so

%.so	: %.o
	$(CC) -shared $^ -lm -o $@


all: $(ALL)

plane: plane.o

honeycomb: honeycomb.o
	$(CC) $^ $(GTSLIBS) $(GLXLIBS) -lm -lglut -o $@

test: test.o
	$(CC) $^ $(GTSLIBS) $(GLXLIBS) -lm -lglut -o $@

ex01	: ex01.o
	$(CC) $(XCBLDLIBS) $^ -o $@

ex02	: ex02.o
	$(CC) $(XCBLDLIBS) $^ -o $@

ex03	: ex03.o
	$(CC) $^ -lxcb -lGL -lEGL -o $@

ex04	: ex04.o
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
	rm -rf *.o *~ $(ALL)
