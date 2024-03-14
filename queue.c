// GRR1255 Gabriel Lisboa Conegero
#include <stdio.h>
#include "queue.h"

int queue_size (queue_t *queue){
    queue_t *first = queue;
    int count = 0;

    if (queue == NULL)
        return count;

    count++;
    for(queue = queue->next; queue != first; queue = queue->next)
        count++;

    return count;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ){
    queue_t *first = queue;

    printf("%s: [", name);

    if (queue != NULL){
        while(queue->next != first){
            print_elem((void*)(queue));
            printf(" ");
            queue = queue->next;
        }
        print_elem((void*)(queue));
    }

    printf("]\n");
}

int queue_append (queue_t **queue, queue_t *elem){
    // Verifica ERROS
    if (queue == NULL){
        fprintf(stderr, "ERRO(queue): Fila deve existir para inserir elemento\n");
        return -1;
    }

    if (elem == NULL){
        fprintf(stderr, "ERRO(queue): Elemento para inserção deve existir\n");
        return -1;
    }

    if ((elem->next != NULL) || (elem->prev != NULL)){
        fprintf(stderr, "ERRO(queue): Elemento não pode pertencer a uma fila para ser inserido\n");
        return -1;
    }
    // Verifica ERROS

    if (*queue == NULL){
        *queue = elem;
        elem->prev = elem;
        elem->next = elem;

        return 0;
    }

    elem->prev = (*queue)->prev;
    elem->prev->next = elem;
    (*queue)->prev = elem;
    elem->next = (*queue);

    return 0;
}

static int queue_has(queue_t *queue, queue_t *elem){
    queue_t *aux = queue;
    if (queue == NULL || elem == NULL || (elem->next == NULL) || (elem->prev == NULL))
        return 0;

    do{
        if (elem == queue)
            return 1;
    }while ((queue = queue->next) != aux);

    return 0;
}

int queue_remove (queue_t **queue, queue_t *elem){
    // Verifica ERROS
    if (queue == NULL){
        fprintf(stderr, "ERRO(queue): Fila deve existir para retirar elemento\n");
        return -1;
    }

    if (elem == NULL){
        fprintf(stderr, "ERRO(queue): Elemento para remoção deve existir\n");
        return -1;
    }

    if (*queue == NULL){
        fprintf(stderr, "ERRO(queue): Fila vazia, não foi possível remover elemento\n");
        return -1;
    }
    // Verifica ERROS
    if (!queue_has(*queue, elem)){
        fprintf(stderr, "ERRO(queue): Elemento não pertence a fila indicada\n");
        return -1;
    }
    
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    if (*queue == elem)
        *queue = elem->next;
    elem->prev = NULL;
    elem->next = NULL;

    // Retirou ultimo elemento
    if ((*queue)->next == NULL)
        *queue = NULL;

    return 0;
}
