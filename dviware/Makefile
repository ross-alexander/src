CC	= g++ -g -Wall
CXX	= g++ -g -Wall
CFLAGS	= `pkg-config kpathsea cairomm-1.16 gtkmm-4.0 gtk4 --cflags`
LDFLAGS	= `pkg-config kpathsea gtk4 --libs-only-L`
LDLIBS	= `pkg-config kpathsea cairomm-1.16 gtk4 gtkmm-4.0 --libs-only-l`
CXXFLAGS = $(CFLAGS)

all:	readchar dviware tt 

tt:	tt.o

dviware: stream.o pk.o cairo.o font.o dviware.o

dviware.o : dviware.cc
	$(CC) $(CFLAGS) -c $^ -o $@

readchar: readchar.o

clean:
	rm -f *.o *~
