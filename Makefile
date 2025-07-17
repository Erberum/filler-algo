all: build run
build:
	gcc main.c -o main.out -Ofast
run:
	./main.out
debug:
	gcc -g main.c -o main.out
	gdb main.out