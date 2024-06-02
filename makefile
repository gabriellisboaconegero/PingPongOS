CFLAGS = -Wall -Wextra
PROJETO = p9
LIBS = obj/queue.o obj/ppos_ipc.o

all: obj exe testes/ping-pong-prodcons testes/ping-pong-prodcons-v2

debug: CFLAGS+= -DDEBUG -g
debug: all

# ============ testes ============
obj/ping-pong-tasks%.o: testes/ping-pong-tasks%.c
	gcc $(CFLAGS) -I $$PWD -c $^ -o $@

testes/ping-pong-tasks%: obj/ping-pong-tasks%.o obj/ppos_core.o $(LIBS)
	gcc $^ -o $@

obj/testafila.o: testes/testafila.c
	gcc $(CFLAGS) -I $$PWD -c $^ -o $@

testes/testafila: obj/testafila.o obj/queue.o
	gcc $^ -o $@

obj/ping-pong-dispatcher.o: testes/ping-pong-dispatcher.c
	gcc $(CFLAGS) -I $$PWD -c $^ -o $@

testes/ping-pong-dispatcher: obj/ping-pong-dispatcher.o obj/ppos_core.o $(LIBS)
	gcc $^ -o $@

obj/ping-pong-scheduler.o: testes/ping-pong-scheduler.c
	gcc $(CFLAGS) -I $$PWD -c $^ -o $@

testes/ping-pong-scheduler: obj/ping-pong-scheduler.o obj/ppos_core.o $(LIBS)
	gcc $^ -o $@

obj/ping-pong-%.o: testes/ping-pong-%.c
	gcc $(CFLAGS) -I $$PWD -c $^ -o $@

testes/ping-pong-%: obj/ping-pong-%.o obj/ppos_core.o $(LIBS)
	gcc $^ -o exe/"`basename $@`"
# ============ testes ============

# ============ objects ============
obj:
	mkdir -p obj

exe:
	mkdir -p exe

obj/queue.o: queue.c queue.h
	gcc $(CFLAGS) -c queue.c -o $@

obj/ppos_core.o: ppos_core.c ppos.h ppos_data.h 
	gcc $(CFLAGS) -c ppos_core.c -o $@

obj/ppos_ipc.o: ppos_ipc.c
	gcc $(CFLAGS) -c ppos_ipc.c -o $@

# ============ objects ============

# ============ misc ============
clean:
	rm -rf exe/* obj/* testes/ping-pong-tasks{1..3} testes/testafila testes/ping-pong-dispatcher testes/ping-pong-scheduler

build: all
	tar --exclude=exe --exclude=testes --exclude=obj --exclude=.git -czvf $(PROJETO).tar.gz *
# ============ misc ============
