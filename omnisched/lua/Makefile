LEX	= flex
CFLAGS	= -Wall -g -I/usr/include/lua/5.4 -fPIC -DLUA_COMPAT_5_1 -DLUA_COMPAT_5_2 -DLUA_COMPAT_5_3
LDLIBS	= -L/usr/lib64/lua/5.4 -llua -lm -ldl

ALL	: holidays date.so

date.so	: time.o tokenize.o parse.o date.o
	$(CC) -shared $^ -o $@

holidays : parse.o tokenize.o time.o holidays.o

holidays.o : holidays.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

parse.o : parse.c parse.h common.h
	$(CC) $(CFLAGS) -c $< -o $@

parse.c parse.h : parse.y
	$(YACC) -d -o $@ $<

tokenize.o : tokenize.c parse.h common.h

tokenize.c tokenize.h : tokenize.lex
	$(LEX) -o $@ --header-file=tokenize.h $<
