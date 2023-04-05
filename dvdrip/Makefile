LDLIBS = -ldvdread -llua
CC = g++ -g
CXX = g++ -g

dvdrip: lua_wrap.o dvdrip.o

dvdrip.o: dvdrip.cc

lua_wrap.cc : dvdrip.i
	swig -c++ -lua -o $@ $<
