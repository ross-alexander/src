LUALIB		= -L/usr/lib64/lua/5.5 -llua
LUAINC		= -I/usr/include/lua/5.5
LDLIBS		=  $(LUALIB)
JSONCONSINC	= -Ijsoncons-1.5.0/include
CXXFLAGS	= -g -std=c++17 -fPIC -D_DEFAULT_SOURCE $(JSONCONSINC) $(LUAINC)
CXX		= g++
CC		= gcc
CFLAGS		= -std=c23 -Wall -fPIC -D_DEFAULT_SOURCE $(JSONCONSINC) $(LUAINC)

ALL		= jlog NetAddr.so luahacks.so

all	: $(ALL)

jlog : parse.tab.o lex.yy.o collector.o collect.o main.o
	$(CXX) $^ -rdynamic $(LDLIBS) -o $@

lex.yy.o: lex.yy.c
	$(CXX) $(CXXFLAGS) -c $^ -o $@

parse.tab.o: parse.tab.c
	$(CXX) $(CXXFLAGS) -c $^ -o $@

parse.tab.c parse.tab.h : parse.y
	bison -d parse.y

lex.yy.c : lex.l
	flex lex.l

collector.o: collector.cc common.h

collect.o: collect.cc common.h

main.o: main.cc common.h

NetAddr.so : netaddr.o
	$(CXX) -shared $< -o $@

netaddr.o: netaddr.cc

luahacks.so: luahacks.o
	$(CXX) -shared $< -o $@

clean:
	rm -f *.o *~ $(ALL) lex.yy.c parse.tab.c parse.tab.h

