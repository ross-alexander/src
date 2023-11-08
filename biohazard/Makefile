CXXFLAGS = -D_GNU_SOURCE `pkg-config --cflags gtk+-3.0 cairo freetype2 fontconfig json-c`
CXX	= g++ -std=c++17
LDLIBS	= `pkg-config --libs gtk+-3.0 cairo freetype2 fontconfig json-c`

ALL	: bio.png bio.svg bio-gtk

bio.png : bio.svg
	rsvg-convert --format=png $< > $@

bio.svg: bio-svg defaults.js
	./bio-svg defaults.js # bio.js

construct.svg: construct.pl
	perl construct.pl

bio-svg: bio.o svg.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

bio-gtk: bio.o gtk.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

typetest: typetest.o

clean:
	rm -f *.o *~ $(ALL)
