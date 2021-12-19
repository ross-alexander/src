CFLAGS = -I.

eval: parse.tab.o tokenize.o main.o
	$(CC) $^ -o $@

dyn.c : dyn.dasc
	dynasm/minilua dynasm/dynasm.lua -D X64 -o $@ $<

parse.tab.c parse.tab.h : parse.y
	bison -d -t $<

tokenize.h tokenize.c: tokenize.lex
	flex --outfile tokenize.c --header-file=tokenize.h $<
