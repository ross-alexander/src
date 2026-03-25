LEX		= reflex --flex --bison --noyywrap
CXX		= g++ -std=c++23
CC		= g++ -std=c++23
CXXFLAGS	= -D_DEFAULT_SOURCE $(shell pkg-config --cflags lua-5.5)
LDLIBS		= -lreflex $(shell pkg-config --libs lua-5.5)

YACC	= bison

roller: lex.yy.o parse.tab.o eval.o main.o
	$(CXX) $^ $(LDLIBS) -o $@

main.o : main.cc common.h

eval.o: eval.cc common.h

parse.tab.h parse.tab.c: parse.y
	$(YACC) -H $<

lex.yy.o: lex.yy.cpp lex.yy.h parse.tab.h

lex.yy.h lex.yy.cpp : lex.l
	$(LEX) --header-file=lex.yy.h $<

clean:
	rm -f *.o *~ lex.yy.h lex.yy.c parse.tab.h parse.tab.c roller
