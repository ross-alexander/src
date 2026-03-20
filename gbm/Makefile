CC	= gcc
CXX	= g++

CFLAGS	= $(shell pkg-config --cflags libdrm) -std=c23 -D_GNU_SOURCE -g
LDLIBS	= $(shell pkg-config --libs libdrm) -lEGL -lGL -lgbm

EXE = example-1 example-2 example-3 example-4 example-5 example-6 formats drm drm-gbm draw

ALL: $(EXE)

drm:	drm.o
	$(CC) -o $@ $< $(LDLIBS)

drm-gbm: drm-gbm.o
	$(CC) -o $@ $< $(LDLIBS)

drm-gbm.o: drm-gbm.c
	$(CC) $(CFLAGS) -c drm-gbm.c 

example-0: example-0.o
	$(CXX) $< -lgbm -o $@

example-0.o: example-0.cc
	$(CXX) -c $< -o $@

example-2: example-2.c
	$(CC) $(CFLAGS) -o $@ $< -ldrm -lgbm -lEGL -lGL -lGLU

example-3: example-3.o
	$(CC) $(CFLAGS) -o $@ $< -ldrm -lgbm -lEGL -lGL -lGLU

example-4: example-4.o
	$(CC) $(CFLAGS) -o $@ $< -ldrm -lgbm -lEGL -lGL -lGLU

draw: draw.o
	$(CC) $< -ldrm -lgbm `pkg-config --libs cairo gegl-0.4` -o $@

draw.o: draw.c
	$(CC) $(CFLAGS) `pkg-config --cflags cairo gegl-0.4` -c $^ -o $@

clean:
	rm -f $(EXE) *.o *~
