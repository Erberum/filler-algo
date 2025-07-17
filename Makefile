all: build run
build:
	gcc main.c -o main.out -O2
run:
	./main.out
debug:
	gcc -g main.c -o main.out
	gdb main.out