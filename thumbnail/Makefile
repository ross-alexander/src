CXXFLAGS = -std=c++2a `pkg-config --cflags gegl-0.4 gtkmm-3.0 cairomm-1.0 fmt libxml++-3.0 libcroco-0.6 lua-5.4`
CFLAGS =  -std=c18 `pkg-config --cflags gdk-pixbuf-2.0 gdk-3.0 gegl-0.4 cairo`
LDLIBS = `pkg-config --libs gtkmm-3.0 cairomm-1.0 gegl-0.4 fmt libxml++-3.0 libcroco-0.6 lua-5.4` -lboost_program_options
ALL = gegl-scale croco++ selection croco-stylesheet thumbnail path-example csheet croco-stylesheet gegl-scale path-example

ALL: $(ALL)

gegl-scale: gegl-scale.o
	$(CXX) $^ $(LDLIBS) -o $@

thumbnail: image.o path.o lua.o thumbnail.o
	$(CXX) $^ $(LDLIBS) -o $@

image.o: image.cc image.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

lua.o: lua.cc image.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

thumbnail.o: thumbnail.cc image.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

csheet: image.o csheet.o
	$(CXX) $^ $(LDLIBS) -o $@

csheet.o : csheet.cc

sac-example-2: sac-example-2.c
	gcc -g  -Wall -o sac-example-2 `pkg-config --cflags libcroco-0.6`  `pkg-config --libs libcroco-0.6` sac-example-2.c

selection: selection.cc
	g++ -g  -Wall -o $@ `pkg-config --cflags --libs libcroco-0.6` $^

croco++: croco++.cc
	g++ -std=c++2a $^ `pkg-config --cflags --libs libcroco-0.6` -o $@

croco-stylesheet: croco-stylesheet.cc
	g++ -std=c++2a $^ `pkg-config --cflags --libs libxml++-3.0 libcroco-0.6` -o $@

clean:
	rm -f *~ *.o $(ALL)
