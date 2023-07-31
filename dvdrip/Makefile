LDLIBS = -ldvdread -llua  $(shell pkg-config --libs glibmm-2.68)
CC = g++ -g
CXX = g++ -g -std=c++20 $(shell pkg-config --cflags glibmm-2.68)

dvdrip: lua_wrap.o dvdrip.o

dvdrip.o: dvdrip.cc

lua_wrap.cc : dvdrip.i
	swig -c++ -lua -o $@ $<

clean:
	rm lua_wrap.o dvdrip.o lua_wrap.cc dvdrip *~

