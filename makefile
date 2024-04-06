CFLAGS = -Wall -Wextra -std=c99

all: obj testes/ping-pong-scheduler

debug: CFLAGS+= -DDEBUG -g
debug: all

# ============ testes ============
obj/ping-pong-tasks%.o: testes/ping-pong-tasks%.c
	gcc $(CFLAGS) -I $$PWD -c $^ -o $@

testes/ping-pong-tasks%: obj/ping-pong-tasks%.o obj/ppos_core.o obj/queue.o
	gcc $^ -o $@

obj/testafila.o: testes/testafila.c
	gcc $(CFLAGS) -I $$PWD -c $^ -o $@

testes/testafila: obj/testafila.o obj/queue.o
	gcc $^ -o $@

obj/ping-pong-dispatcher.o: testes/ping-pong-dispatcher.c
	gcc $(CFLAGS) -I $$PWD -c $^ -o $@

testes/ping-pong-dispatcher: obj/ping-pong-dispatcher.o obj/ppos_core.o obj/queue.o
	gcc $^ -o $@

obj/ping-pong-scheduler.o: testes/ping-pong-scheduler.c
	gcc $(CFLAGS) -I $$PWD -c $^ -o $@

testes/ping-pong-scheduler: obj/ping-pong-scheduler.o obj/ppos_core.o obj/queue.o
	gcc $^ -o $@
# ============ testes ============

obj:
	mkdir -p obj

obj/queue.o: queue.c queue.h
	gcc $(CFLAGS) -c queue.c -o $@

obj/ppos_core.o: ppos_core.c ppos.h ppos_data.h 
	gcc $(CFLAGS) -c ppos_core.c -o $@

clean:
	rm -rf obj/* testes/ping-pong-tasks{1..3} testes/testafila testes/ping-pong-dispatcher testes/ping-pong-scheduler

build: all
	tar --exclude=testes --exclude=obj --exclude=.git -czvf p4.tar.gz *
