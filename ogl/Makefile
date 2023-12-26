.suffix: .so

%.so	: %.o
	$(CC) -shared $^ -o $@


CFLAGS	= -std=c99 -fPIC -D_XOPEN_SOURCE_EXTENDED -D_GNU_SOURCE `pkg-config --cflags gtkgl-2.0`
LDFLAGS	= `pkg-config --libs gtkgl-2.0` -lGLU -lglut -ldl -L/usr/X11/lib -lX11 -lXmu -lXext -lXi

ALL =  gtkgl dl cube.so smooth.so aapoly.so scenebamb.so colormat.so bezmesh.so accanti.so ncube.so polyoff.so molehill

all: $(ALL)

gtkgl	: gtkgl.o

cube.so	: cube.o

smooth.so	: smooth.o


molehill: molehill.o
	$(CC) $< -lGL -lGLU -lglut -o $@

clean:
	rm -f *.o *.so *~ $(ALL)
