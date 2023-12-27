LUALIB		= -L/usr/lib64 -llua
LUAINC		= -I/usr/include
LDLIBS		=  $(LUALIB)
JSONCONSINC	= -Ijsoncons-0.172.1/include
CXXFLAGS	= -g -std=c++11 -fPIC -D_DEFAULT_SOURCE -D_POSIX_SOURCE $(JSONCONSINC) $(LUAINC)
CXX		= g++

ALL		= jlog NetAddr.so

all	: $(ALL)

jlog : parse.tab.o lex.yy.o collector.o collect.o main.o
	$(CXX) $^ -rdynamic $(LDLIBS) -o $@

lex.yy.o: lex.yy.c
	$(CXX)  $(CXXFLAGS) -c $^ -o $@

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

clean:
	rm -f *.o *~ $(ALL) lex.yy.c parse.tab.c parse.tab.h

