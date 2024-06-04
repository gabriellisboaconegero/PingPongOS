// Gabriel Lisboa Conegero - GRR20221255
#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "ppos_data.h"
#define PPOS_KERNEL_BUSY_CS
#include "ppos_kernel_funcs.h"
#undef PPOS_KERNEL_BUSY_CS

extern task_t *TaskCurr ;
extern task_t *TaskDispatcher ;
extern task_t *ReadyQueue ;

int kernel_sem_init (semaphore_t *s, int value) {
    if (s == NULL || s->dead)
        return PPOS_SEM_ERROR ;
    s->cont = value ;
    s->task_queue = NULL ;
    s->dead = 0 ;
    return PPOS_SEM_OK ;
}

int kernel_sem_down (semaphore_t *s) {
    if (s == NULL || s->dead)
        return PPOS_SEM_ERROR ;
    sem_enter_cs(&s->lock) ;
    s->cont-- ;
#ifdef DEBUG
    PPOS_DEBUG("Task %d fez down em semaforo %p: %d", task_id(), s, s->cont) ;
#endif
    if (s->cont < 0){
#ifdef DEBUG
        PPOS_DEBUG("Task %d suspensa (%d)", task_id(), systime()) ;
#endif
        // Se task->prev não for NULL quer dizer que está em uma fila, 
        // Se for NULL quer dizer que não está em uma fila
        if (TaskCurr->prev != NULL && queue_remove((queue_t **) &ReadyQueue, (queue_t *) TaskCurr) < 0)
            exit(1) ;
        TaskCurr->status = PPOS_SUSPENSA ;
        if (queue_append((queue_t **)&s->task_queue, (queue_t *) TaskCurr) < 0)
            exit(1) ;
        // Apoś alterar fila pode sair da zona critica
        sem_leave_cs(&s->lock) ;
        kernel_task_switch(TaskDispatcher) ;
    }
    sem_leave_cs(&s->lock) ;
    // Se o semaforo tiver sido destruido retornar erro
    return s->dead ? PPOS_SEM_ERROR : PPOS_SEM_OK ;
}

int kernel_sem_up (semaphore_t *s) {
    if (s == NULL || s->dead)
        return PPOS_SEM_ERROR ;
    (s->cont)++ ;
#ifdef DEBUG
    PPOS_DEBUG("Task %d fez up em semaforo %p: %d", task_id(), s, s->cont) ;
#endif
    if (s->cont <= 0)
        kernel_task_awake(s->task_queue, &(s->task_queue)) ;
    return PPOS_SEM_OK ;
}

int kernel_sem_destroy (semaphore_t *s) {
    if (s == NULL || s->dead)
        return PPOS_SEM_ERROR ;
    s->dead = 1 ;
    while(s->task_queue != NULL)
        kernel_task_awake(s->task_queue, &s->task_queue) ;
    return PPOS_SEM_OK ;
}
