SRC = $(wildcard src/*.c) $(wildcard src/**/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/**/**/*.c)
INCLUDE = $(wildcard include/*.c) $(wildcard include/**/*.c) $(wildcard include/**/**/*.c) $(wildcard include/**/**/**/*.c)
PROTOCOL_LOCATION = /run/current-system/sw/share/wayland-protocols/stable/xdg-shell/

PKGS = -l wayland-client -l xkbcommon -l fontconfig

dir:
	mkdir -p build

main: dir src/main.c
	gcc -std=c11 -D_POSIX_C_SOURCE=200809L -o build/main $(SRC) $(INCLUDE) $(PKGS) -lm -g -O2

xdg-shell:
	wayland-scanner client-header $(PROTOCOL_LOCATION)xdg-shell.xml include/xdg-shell.h
	wayland-scanner private-code $(PROTOCOL_LOCATION)xdg-shell.xml include/xdg-shell.c

wlr-layer-shell:
	wayland-scanner client-header protocols/wlr-layer-shell-unstable-v1.xml include/wlr-layer-shell.h
	wayland-scanner private-code protocols/wlr-layer-shell-unstable-v1.xml include/wlr-layer-shell.c

run: xdg-shell wlr-layer-shell main
	build/main

clean:
	rm -rf build
	rm include/xdg-shell*
	rm include/wlr-layer-shell*
