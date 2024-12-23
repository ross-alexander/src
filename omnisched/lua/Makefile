LEX		= flex
YACC		= bison
CFLAGS		= -g -I/usr/include/lua/5.4 -fPIC -DLUA_COMPAT_5_1 -DLUA_COMPAT_5_2 -DLUA_COMPAT_5_3
LDLIBS		= -L/usr/lib64/lua/5.4 -llua -lm -ldl
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

