CFLAGS = -I.

eval: parse.tab.o tokenize.o expr.o main.o
	$(CC) $^ -o $@

main.o: main.c eval.h tokenize.h parse.tab.h
	$(CC) -c $(CFLAGS) $< -o $@

dyn.c : dyn.dasc
	dynasm/minilua dynasm/dynasm.lua -D X64 -o $@ $<

parse.tab.o: parse.tab.c tokenize.h eval.h
	$(CC) -c $(CFLAGS) $< -o $@

parse.tab.c parse.tab.h : parse.y
	bison -d -t $<

tokenize.h tokenize.c: tokenize.lex
	flex --outfile tokenize.c --header-file=tokenize.h $<
