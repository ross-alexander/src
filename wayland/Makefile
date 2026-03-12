CC	= gcc -g -Wall
CXX	= g++ -g -Wall

PKG_CONFIG ?= pkg-config

WAYLAND_PROTOCOLS_DIR = $(shell $(PKG_CONFIG) wayland-protocols --variable=pkgdatadir)
WAYLAND_SCANNER = $(shell pkg-config --variable=wayland_scanner wayland-scanner)
XDG_SHELL_PROTOCOL = $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml
CAIRO_CFLAGS = $(shell $(PKG_CONFIG) cairo --cflags)
CAIRO_LIBS = $(shell $(PKG_CONFIG) cairo --libs)
BABL_CFLAGS = $(shell $(PKG_CONFIG) babl-0.1 --cflags)
BABL_LIBS = $(shell $(PKG_CONFIG) babl-0.1 --libs)
GLYCIN_CFLAGS = $(shell $(PKG_CONFIG) glycin-2 --cflags)
GLYCIN_LIBS = $(shell $(PKG_CONFIG) glycin-2 --libs)
LUA_CFLAGS = -Ilua/include
LUA_LIBS = -Llua/lib -llua -lm

ALL: connect registry shell surface client cairo surfers wt_example

client.o: client.c xdg-shell-client-protocol.h
	$(CC) -g -o $@ -c $<  $(CAIRO_CFLAGS) $(LUA_CFLAGS) $(BABL_CFLAGS) $(GLYCIN_CFLAGS)

client: xdg-shell-client.o client.o
	$(CC) -g -o $@ $^ $(CAIRO_LIBS) $(LUA_LIBS) $(BABL_LIBS) $(GLYCIN_LIBS) -lwayland-client

wt_example: w.c
	$(CXX) -o $@ $< xdg-shell-client.o -lwayland-client

surfers: surfers.cc
	$(CXX) $(CAIRO_CFLAGS) -o $@ $< xdg-shell-client.o -lwayland-client $(CAIRO_LIBS)

connect: connect.c
	$(CC) -o $@ $< -lwayland-client

registry: registry.c
	$(CC) -o $@ $< -lwayland-client

shell: shell.c
	$(CC) -o $@ $< -lwayland-client

surface: surface.c
	$(CC) -o $@ $< -lwayland-client

xdg-shell-client.o: xdg-shell-client.c xdg-shell-client-protocol.h
	$(CC) -g -o $@ -c $<  $(CAIRO_FLAGS)

cairo: cairo.c
	$(CC) -g -o $@ $^ $(CAIRO_CFLAGS) $(CAIRO_LIBS)

xdg-shell-client-protocol.h:
	$(WAYLAND_SCANNER) client-header $(XDG_SHELL_PROTOCOL) xdg-shell-client-protocol.h

xdg-shell-client.c:
	$(WAYLAND_SCANNER) private-code $(XDG_SHELL_PROTOCOL) xdg-shell-client.c
