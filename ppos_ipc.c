// Gabriel Lisboa Conegero - GRR20221255
#include <stdio.h>
#include "ppos.h"
#include "ppos_kernel_funcs.h"

#define PPOS_DEBUG(msg, ...) printf("PPOS[%s]: "msg"\n", __func__, __VA_ARGS__)
#define PPOS_SEM_ERROR -1
#define PPOS_SEM_OK 0

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
    s->cont-- ;
#ifdef DEBUG
    PPOS_DEBUG("Semaforo %p down: %d", s, s->cont) ;
#endif
    if (s->cont < 0)
        kernel_task_suspend(&(s->task_queue)) ;
    // Se o semaforo tiver sido destruido retornar erro
    return s->dead ? PPOS_SEM_ERROR : PPOS_SEM_OK ;
}

int kernel_sem_up (semaphore_t *s) {
    if (s == NULL || s->dead)
        return PPOS_SEM_ERROR ;
    (s->cont)++ ;
#ifdef DEBUG
    PPOS_DEBUG("Semaforo %p up: %d", s, s->cont) ;
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
