all: build run
build:
	gcc main.c -o main -march=native -O3 -funroll-loops
run:
	./main