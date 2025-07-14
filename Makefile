all: build run
build:
	gcc main.c -o main.out -march=native -O3 -funroll-loops
run:
	./main.out