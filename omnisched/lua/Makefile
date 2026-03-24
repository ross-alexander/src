CC		= gcc -std=gnu23 -Wall
LEX		= flex
YACC		= bison
CFLAGS		= -Wall -g -fPIC $(shell pkg-config --cflags lua-5.5)
LDLIBS		= $(shell pkg-config --libs lua-5.5)
OBJS		= time.o tokenize.o parse.o date.o date.so
GENERATED	= tokenize.c tokenize.h parse.h parse.c

ALL	: holidays date.so

date.so	: time.o tokenize.o parse.o date.o
	$(CC) -shared $^ -o $@

holidays : parse.o tokenize.o time.o date.o holidays.o

holidays.o : holidays.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

parse.o : parse.c parse.h tokenize.h common.h
	$(CC) $(CFLAGS) -c $< -o $@

parse.c parse.h &: parse.y
	$(YACC) -d -o parse.c $<

tokenize.o : tokenize.c parse.h common.h

tokenize.c tokenize.h &: tokenize.lex
	$(LEX) -o tokenize.c --header-file=tokenize.h $<

clean:
	rm -f $(OBJS) $(GENERATED)

