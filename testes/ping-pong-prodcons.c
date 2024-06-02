#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

typedef struct item_t {
    struct item_t *prev, *next ;
    int val ;
} item_t ;

#define SLEEP 1000
#define true 1

task_t con1, con2 ;
task_t prod1, prod2, prod3 ;
item_t *buffer ;
semaphore_t s_vaga, s_buffer, s_item ; 

item_t *prod_item(int val){
    item_t *i = calloc(1, sizeof(item_t)) ;
    if (i != NULL)
        i->val = val ;
    return i ;
}

void produtor(void *arg){
    item_t *item ;
    while(true){
        task_sleep(SLEEP) ;
        if ((item = prod_item(random() % 100)) == NULL)
            exit(1) ;
        // Quer vaga para produzir
        sem_down(&s_vaga) ;
        sem_down(&s_buffer) ;
        if (queue_append((queue_t **) &buffer, (queue_t *) item) < 0)
            exit(1) ;
        sem_up(&s_buffer) ;
        // Libera item produzido
        sem_up(&s_item) ;
        printf("%s produziu: %d (tem %d)\n", (char *)arg, item->val, queue_size((queue_t *)buffer)) ;
    }
}

void consumidor(void *arg){
    item_t *item ;
    while(true){
        sem_down(&s_item) ;
        sem_down(&s_buffer) ;
        item = buffer ;
        if (queue_remove((queue_t  **)&buffer, (queue_t *)item) < 0)
            exit(1) ;
        sem_up(&s_buffer) ;
        sem_up(&s_vaga) ;

        printf("%s consumiu %d (tem %d)\n", (char *)arg, item->val, queue_size((queue_t *)buffer)) ;
        free(item) ;
        task_sleep(SLEEP) ;
    }
}

int main(){
    printf("main: inicio\n") ;
    ppos_init() ;

    sem_init(&s_buffer, 1) ;
    sem_init(&s_vaga, 5) ;
    sem_init(&s_item, 0) ;

    task_init(&con1, consumidor, "\t\t\t\tC1") ;
    task_init(&con2, consumidor, "\t\t\t\tC2") ;
    task_init(&prod1, produtor, "P1") ;
    task_init(&prod2, produtor, "P2") ;
    task_init(&prod3, produtor, "P3") ;

    task_exit(0) ;
}
