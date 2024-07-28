CFLAGS = -Wall -Wextra
PROJETO = p12
LIBS = obj/queue.o obj/ppos_ipc.o obj/ppos_mqueue.o obj/disk.o obj/ppos_disk.o
LLIBS = -lm -lrt

# all:   CFLAGS += -DDEBUG_LOCK
all: obj exe disk testes/ping-pong-disco2

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
	gcc $^ -o exe/"`basename $@`" $(LLIBS)
# ============ testes ============

# ============ objects ============
obj:
	mkdir -p obj

exe:
	mkdir -p exe
	ln -s disk.dat exe/disk.dat

disk:
	cp disk.dat.bkp disk.dat

obj/queue.o: queue.c queue.h
	gcc $(CFLAGS) -c queue.c -o $@

obj/ppos_core.o: ppos_core.c ppos.h ppos_data.h 
	gcc $(CFLAGS) -c ppos_core.c -o $@

obj/ppos_ipc.o: ppos_ipc.c ppos.h ppos_data.h
	gcc $(CFLAGS) -c ppos_ipc.c -o $@

obj/ppos_mqueue.o: ppos_mqueue.c ppos.h ppos_data.h
	gcc $(CFLAGS) -c ppos_mqueue.c -o $@

obj/disk.o: disk.c disk.h
	gcc $(CFLAGS) -c disk.c -o $@

obj/ppos_disk.o: ppos_disk.c ppos_disk.h
	gcc $(CFLAGS) -c ppos_disk.c -o $@

# ============ objects ============

# ============ misc ============
clean:
	rm -rf exe/* obj/* testes/ping-pong-tasks{1..3} testes/testafila testes/ping-pong-dispatcher testes/ping-pong-scheduler

build: all
	tar --exclude=exe --exclude=testes --exclude=obj --exclude=.git -czvf $(PROJETO).tar.gz *
# ============ misc ============
