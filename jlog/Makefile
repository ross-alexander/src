LDLIBS		= -Llua-5.4.6/lib -llua -ldl
CXXFLAGS	= -g -std=c++11 -fPIC -D_BSD_SOURCE -D_POSIX_SOURCE -Ijsoncons-0.172.1/include -Ilua-5.4.0/include
CFLAGS		= $(CXXFLAGS)
CXX		= g++
CC		= g++

ALL		= jlog NetAddr.so

all	: $(ALL)

jlog : parse.tab.o lex.yy.o collector.o collect.o main.o
	$(CXX) $^ -rdynamic $(LDLIBS) -o $@

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

