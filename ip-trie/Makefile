CFLAGS	= -g -std=c17 `pkg-config --cflags glib-2.0`
LDLIBS	= `pkg-config --libs glib-2.0`

trie	: trie.o

clean:
	rm -f *.o *~ trie
