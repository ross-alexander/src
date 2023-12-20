CC	= gcc
CXX	= g++

CFLAGS	=  `pkg-config --cflags libdrm` -std=c17 -D_GNU_SOURCE -g
LDLIBS	= `pkg-config --libs libdrm` -lEGL -lGL -lgbm

EXE = example-1 example-2 example-3 example-4 example-5 example-6 formats drm example-0 drm-gbm draw

ALL: $(EXE)

drm-gbm: drm-gbm.c
	gcc -o drm-gbm -I/usr/include/libdrm drm-gbm.c -ldrm -lgbm -lEGL -lGL

example-0: example-0.o
	$(CXX) $< -lgbm -o $@

example-0.o: example-0.cc
	$(CXX) -c $< -o $@

example-4: example-4.o
	gcc $(CFLAGS) -o $@ $< -ldrm -lgbm -lEGL -lGL -lGLU

example-3: example-3.o

example-2: example-2.c
	gcc $(CFLAGS) -o $@ $< -ldrm -lgbm -lEGL -lGL -lGLU

draw: draw.o
	$(CC) $< -ldrm -lgbm `pkg-config --libs cairo gegl-0.4` -o $@

draw.o: draw.c
	$(CC) $(CFLAGS) `pkg-config --cflags cairo gegl-0.4` -c $^ -o $@

clean:
	rm -f $(EXE) *.o *~
