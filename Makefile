all:
	gcc -std=c99 -O0 -ggdb3 -o main main.c integral.c basic.c -lm