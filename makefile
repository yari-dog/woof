SRC = $(wildcard src/*.c) $(wildcard src/**/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/**/**/*.c)

dir:
	mkdir -p build

main: dir src/main.c
	gcc -o build/main $(SRC)

run: main
	build/main
