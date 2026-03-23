ALL : decode typedef t01.raw double

CC	= gcc -std=c23

decode: decode.o

t01.raw: t01.lua
	luajit -b $< $@

clean:
	rm -f *~ *.o decode
