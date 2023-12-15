LEX	= flex
CC	= g++ -std=c++17
CFLAGS  = -D_POSIX_C_SOURCE
YACC	= bison

roller: lex.yy.o parse.tab.o eval.o main.o
	$(CC) $^ -o $@

main.o : main.cc

parse.tab.h parse.tab.c: parse.y
	$(YACC) -H $<

lex.yy.o: lex.yy.c parse.tab.h

lex.yy.h lex.yy.c : lex.l
	$(LEX) --header=lex.yy.h $<

clean:
	rm *.o *~ lex.yy.h lex.yy.c parse.tab.h parse.tab.c

