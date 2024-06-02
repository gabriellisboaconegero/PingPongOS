// PingPongOS - PingPong Operating System

// Teste de produtores e consumidores 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ppos.h"

task_t      p1, p2, p3, c1, c2;
semaphore_t s_buffer, s_item, s_vaga;
int buffer[5], item, cont;

// corpo da thread producer
void TaskP (void * arg)
{
    while (1) {
        task_sleep(1000);
        item = rand() % 100;

        sem_down(&s_vaga);
        sem_down(&s_buffer);
        // insere item no buffer
        buffer[cont] = item;
        cont++;
        printf("%s produziu %d\n", (char*)arg, item);
        sem_up(&s_buffer);
        sem_up(&s_item);
    }
    task_exit(0);
}

// corpo da thread consumer
void TaskC (void * arg)
{
    while (1) {
        sem_down(&s_item);
        sem_down(&s_buffer);
        // retira item do buffer
        item = buffer[0];
        for(int i = 0; i < cont - 1; i++)
            buffer[i] = buffer[i+1];
        cont--;
        printf("%s consumiu %d\n", (char*)arg, item);
        sem_up(&s_buffer);
        sem_up(&s_vaga);

        // print item
        task_sleep (1000);
    }
    task_exit(0);
}

int main (int argc, char *argv[])
{
   printf ("main: inicio\n") ;

   ppos_init () ;

   // dependencias do buffer
   srand(0);
   cont = 0;

   // inicia semaforos
   sem_init (&s_buffer, 5) ;
   sem_init (&s_item, 0) ;
   sem_init (&s_vaga, 5) ;

   // inicia tarefas
   task_init (&p1, TaskP, "p1") ;
   task_init (&p2, TaskP, "p2") ;
   task_init (&p3, TaskP, "p3") ;
   task_init (&c1, TaskC, "\t\t\t\tc1") ;
   task_init (&c2, TaskC, "\t\t\t\tc2") ;

   // aguarda a2, b1 e b2 encerrarem
   task_wait (&p1) ;

   // destroi semaforos
   sem_destroy (&s_buffer) ;
   sem_destroy (&s_item) ;
   sem_destroy (&s_vaga) ;

   printf ("main: fim\n") ;

   task_exit (0) ;
}

