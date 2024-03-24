CFLAGS= -Wall -Wextra -std=c99 -g

all: testes/ping-pong-tasks1 testes/ping-pong-tasks2 testes/ping-pong-tasks3

debug: CFLAGS+= -DDEBUG
debug: all

testes/ping-pong-tasks%: testes/ping-pong-tasks%.c ppos_core.c
	gcc $(CFLAGS) $^ -o $@

queue: queue.c queue.h
	gcc $(CFLAGS) $< -o $@

testafila: testafila.c queue.c
	gcc $(CFLAGS) $^ -o $@

ppos_core.o: ppos_core.c ppos.h ppos_data.h
	gcc $(CFLAGS) -c ppos_core.c -o $@

clean:
	rm -rf testafila queue testes/ping-pong-tasks{1..3}
