// Gabriel Lisboa Conegero - GRR20221255
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppos.h"
#include "ppos_kernel_funcs.h"

#define PPOS_DEBUG(msg, ...) printf("PPOS[%s]: "msg"\n", __func__, __VA_ARGS__)
#define PPOS_QUEUE_ERROR -1
#define PPOS_QUEUE_OK 0

int mqueue_init (mqueue_t *queue, int max, int size) {
    if (queue == NULL || queue->dead)
        return PPOS_QUEUE_ERROR ;
    queue->max_msgs = max ;
    queue->msg_size = size ;
    queue->queue = NULL ;
    queue->dead = 0 ;
    sem_init(&queue->s_buffer, 1) ;
    sem_init(&queue->s_vaga, max) ;
    sem_init(&queue->s_item, 0) ;
#ifdef DEBUG
    PPOS_DEBUG("MQueue (%p)iniciada com: max %d, size : %d", queue, queue->max_msgs, queue->msg_size) ;
    PPOS_DEBUG("MQueue semaforos:\n\ts_vaga: %p\n\ts_item: %p\n\ts_buffer: %p",&queue->s_vaga, &queue->s_item, &queue->s_buffer) ;
#endif
    return PPOS_QUEUE_OK ;
}

static struct buffer_t *prod_msg(mqueue_t *q, void *msg){
    struct buffer_t *b = calloc(1, sizeof(q->queue)) ;
    if (b != NULL){
        b->msg = calloc(1, q->msg_size) ;
        if (b->msg == NULL){
            free(b) ;
            return (struct buffer_t *)NULL ;
        }
        memcpy(b->msg, msg, q->msg_size) ;
    }
    return b ;
}

int mqueue_send (mqueue_t *queue, void *msg) {
    int ret = PPOS_QUEUE_OK ;
    // Não pode mandar mensagem quando sinal dead ou fila dead
    if (queue == NULL || queue->dead ||
        queue->s_vaga.dead || queue->s_buffer.dead ||
        queue->s_item.dead)
        return PPOS_QUEUE_ERROR ;
    struct buffer_t *m = prod_msg(queue, msg) ;
    if (m == NULL)
        return PPOS_QUEUE_ERROR ;
    // Se semaforo for destruido (ou file, pois destroi o sinal tambem então nao verifica se qeueu->dead)
    if (sem_down(&queue->s_vaga) < 0 || sem_down(&queue->s_buffer) < 0){
        free(m->msg) ;
        free(m) ;
        return PPOS_QUEUE_ERROR ;
    }

    if (queue_append((queue_t **) &queue->queue, (queue_t *)m) < 0){
        free(m->msg) ;
        free(m) ;
        ret = PPOS_QUEUE_ERROR ;
    }

    // Se semaforo for destruido (ou file, pois destroi o sinal tambem então nao verifica se qeueu->dead)
    if (sem_up(&queue->s_buffer) < 0 || sem_up(&queue->s_item) < 0){
        free(m->msg) ;
        free(m) ;
        return PPOS_QUEUE_ERROR ;
    }
    return ret ;
}
int mqueue_recv (mqueue_t *queue, void *msg) {
    int ret = PPOS_QUEUE_OK ;
    struct buffer_t *b ;
    // Não pode mandar mensagem quando sinal dead ou fila dead
    if (queue == NULL || queue->dead || msg == NULL ||
        queue->s_vaga.dead || queue->s_buffer.dead ||
        queue->s_item.dead)
        return PPOS_QUEUE_ERROR ;
    if (sem_down(&queue->s_item) < 0 || sem_down(&queue->s_buffer) < 0)
        return PPOS_QUEUE_ERROR ;
    b = queue->queue ;
    if (queue_remove((queue_t **)&queue->queue, (queue_t *)queue->queue) < 0){
        ret = PPOS_QUEUE_ERROR ;
    }else{
        memcpy(msg, b->msg, queue->msg_size) ;
        free(b->msg) ;
        free(b) ;
    }
    if (sem_up(&queue->s_buffer) < 0 || sem_up(&queue->s_vaga) < 0)
        return PPOS_QUEUE_ERROR ;
    
    return ret ;
}
int mqueue_destroy (mqueue_t *queue) {
    if (queue == NULL || queue->dead)
        return PPOS_QUEUE_ERROR ;
    // Destroi semaforos. Mas antes pega o buffer, para evitar
    // que o buffer seja acessado após ser destruido. Depois
    // destroi os semaforos
    if (sem_down(&queue->s_buffer) < 0 ||
        sem_destroy(&queue->s_buffer) < 0 || 
        sem_destroy(&queue->s_vaga) < 0 ||
        sem_destroy(&queue->s_item) < 0 )
        return PPOS_QUEUE_ERROR ;
    // Sinaliza morte
    queue->dead = 1 ;
    // Retira todos as menssagens restantes
    while(queue->queue != NULL){
        struct buffer_t *aux = queue->queue ;
        if (queue_remove((queue_t **)&queue->queue, (queue_t *)queue->queue) < 0)
            exit(1) ;
        if (aux->msg != NULL)
            free(aux->msg) ;
        free(aux) ;
    }

    return PPOS_QUEUE_OK ;
}
int mqueue_msgs (mqueue_t *queue) {
    int ret ;
    if (queue == NULL || queue->dead)
        return PPOS_QUEUE_ERROR ;
    if (sem_down(&queue->s_buffer) < 0)
        return PPOS_QUEUE_ERROR ;
    ret = queue_size((queue_t *)queue->queue) ;
    if (sem_up(&queue->s_buffer) < 0)
        return PPOS_QUEUE_ERROR ;
    return ret ;
}
