CFLAGS= -Wall -std=c99 -g
queue: queue.c queue.h
	gcc $(CFLAGS) $< -o $@

testafila: testafila.c queue.c
	gcc $(CFLAGS) $^ -o $@
