PKG_CONFIG ?= pkg-config

WAYLAND_PROTOCOLS_DIR = $(shell $(PKG_CONFIG) wayland-protocols --variable=pkgdatadir)
WAYLAND_SCANNER = $(shell pkg-config --variable=wayland_scanner wayland-scanner)
XDG_SHELL_PROTOCOL = $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml
CAIRO_FLAGS = $(shell $(PKG_CONFIG) cairo --cflags --libs)

ALL: connect registry shell surface client cairo

connect: connect.c
	cc -o $@ $< -lwayland-client

registry: registry.c
	cc -o $@ $< -lwayland-client

shell: shell.c
	cc -o $@ $< -lwayland-client

surface: surface.c
	cc -o $@ $< -lwayland-client

client: xdg-shell-client.c client.c
	cc -g -o $@ $^ $(CAIRO_FLAGS) -lwayland-client

cairo: cairo.c
	cc -g -o $@ $^ $(CAIRO_FLAGS)

xdg-shell-client-protocol.h:
	$(WAYLAND_SCANNER) client-header $(XDG_SHELL_PROTOCOL) xdg-shell-client-protocol.h

xdg-shell-client.c:
	$(WAYLAND_SCANNER) private-code $(XDG_SHELL_PROTOCOL) xdg-shell-client.c
