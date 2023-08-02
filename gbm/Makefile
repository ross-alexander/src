CC	= gcc -std=c17 -D_GNU_SOURCE -g
CXX	= g++

CFLAGS	= `pkg-config --cflags libdrm`
LDLIBS	= `pkg-config --libs libdrm` -lEGL -lGL -lgbm

ALL: example-1 example-2 example-3 example-4 example-5 example-6 formats drm example-0 drm-gbm


drm-gbm: drm-gbm.c
	gcc -o drm-gbm -I/usr/include/libdrm drm-gbm.c -ldrm -lgbm -lEGL -lGL

example-0: example-0.o
	$(CXX) $< -lgbm -o $@

example-0.o: example-0.cc
	$(CXX) -c $< -o $@

example-3: example-3.o

example-2: example-2.c
	gcc $(CFLAGS) -o $@ $< -ldrm -lgbm -lEGL -lGL -lGLU
