CXXFLAGS	= -std=c++2a -I. -fPIC -D_DEFAULT_SOURCE -DHAVE_CONFIG_H `pkg-config --cflags x11 xcb xcb-aux cairo gdk-3.0 lua-5.4`
LDLIBS		= `pkg-config --libs x11 xcb xcb-aux cairo gdk-3.0 lua-5.4` -llua -ldl -lm

# CXXFLAGS	= -std=c++2a -I. -fPIC -D_DEFAULT_SOURCE -DHAVE_CONFIG_H `pkg-config --cflags x11 xcb xcb-aux cairo gtk4-x11 lua-5.4`
# LDLIBS		= `pkg-config --libs x11 xcb xcb-aux cairo gtk4-x11 lua-5.4` -llua -ldl -lm

DESTDIR		= /locker/pkg/x86/tile

LUA_LIB_DIR	= /usr/lib64/lua/5.4

ALL = Scanner.so cairobg tile # fs dt

all:  $(ALL)

tile	: tile-xcb.o tile-format.o tile-surface.o tile-pixbuf.o tile-scanner.o tile.o
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -lfmt -o $@

dt	: dt.o duktape.o
	$(CXX) $(CXXFLAGS) $^ `pkg-config --libs cairo` -o $@

dt.o	: dt.cc
	$(CXX) $(CXXFLAGS) -I dukglue/include -I . `pkg-config --cflags cairo` -c $< -o $@

duktape.o: duktape.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

Scanner.so	: scanner.o
	$(CXX) $(LDFLAGS) `pkg-config --libs lua-5.4` -shared $^ -llua -o $@

scanner.o	: scanner.cc

install: Scanner.so
	install $^ $(LUA_LIB_DIR)

pkg-install: tile bg.lua
	install -d $(DESTDIR)/usr/local/bin
	install tile $(DESTDIR)/usr/local/bin
	install -d $(DESTDIR)/usr/local/share/tile
	install bg.lua $(DESTDIR)/usr/local/share/tile

clean:
	rm -f *.o *~ $(ALL)
