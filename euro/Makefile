CC		= g++ -std=c++23
CPPFLAGS	= -Wall $(shell pkg-config --cflags cairomm-1.16)
LDLIBS		= $(shell pkg-config --libs cairomm-1.16) -lm

all:	image.png

image.png	: euro
	./euro

euro	: geometry.o euro.o

euro.o	: euro.cc geometry.h

geometry.o: geometry.cc geometry.h

test: test.o
	$(CC) $< $(LIBS) -o $@

clean:
	rm *.o *~ euro
