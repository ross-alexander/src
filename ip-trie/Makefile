CFLAGS	= -g -Wall -std=c23 -D_DEFAULT_SOURCE `pkg-config --cflags glib-2.0`
LDLIBS	= `pkg-config --libs glib-2.0`

all: trie mask # trie-rs.o

trie	: trie.o

trie-rs.o: trie-rs.rs
	rustc --crate-type=lib --emit=obj $<

clean:
	rm -f *.o *~ trie
