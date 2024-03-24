CFLAGS = -Wall -Wextra -std=c99

all: testes/ping-pong-tasks1 testes/ping-pong-tasks2 testes/ping-pong-tasks3

debug: CFLAGS+= -DDEBUG -g
debug: all

obj/ping-pong-tasks%.o: testes/ping-pong-tasks%.c
	gcc $(CFLAGS) -I $$PWD -c $^ -o $@

testes/ping-pong-tasks%: obj/ping-pong-tasks%.o obj/ppos_core.o
	gcc $^ -o $@

obj/testafila.o: testes/testafila.c
	gcc $(CFLAGS) -I $$PWD -c $^ -o $@

testes/testafila: obj/testafila.o obj/queue.o
	gcc $^ -o $@

obj/queue.o: queue.c queue.h
	gcc $(CFLAGS) -c queue.c -o $@

obj/ppos_core.o: ppos_core.c ppos.h ppos_data.h
	gcc $(CFLAGS) -c ppos_core.c -o $@

clean:
	rm -rf obj/* testes/ping-pong-tasks{1..3} testes/testafila
