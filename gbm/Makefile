CFLAGS	= `pkg-config --cflags libdrm`
LDLIBS	= `pkg-config --libs libdrm` -lEGL -lGL -lgbm

ALL: example-1 example-2 example-3 example-4 example-5 formats

example-3: example-3.o

example-2: example-2.c
	gcc $(CFLAGS) -o $@ $< -ldrm -lgbm -lEGL -lGL -lGLU
