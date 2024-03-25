#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

#define PPOS_TASK_ERROR_CODE -1
#define PPOS_TASK_OK_CODE 0
#define PPOS_STACK_SZ 1<<16
#define PPOS_DEBUG(msg, ...) printf("PPOS[%s]: "msg"\n", __func__, __VA_ARGS__)
#define PPOS_RODANDO 0
#define PPOS_PRONTA 1
#define PPOS_TERMINADA 2

static task_t __TaskDispatcher ;
static task_t __TaskMain ;
static task_t *TaskCurr = &__TaskMain ;
static task_t *TaskQueue ;
static task_t *TaskDispatcher = &__TaskDispatcher ;
static int idCounter = 0 ;

static void dispatcher(void * arg) ;

#ifdef DEBUG
static void print_task (void *ptr) {
   task_t *elem = ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d>", elem->id) ;
   elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
}
#endif

void ppos_init () {
#if DEBUG
    PPOS_DEBUG("%s", "Inicializando PpOS\n") ;
#endif
    setvbuf (stdout, 0, _IONBF, 0) ;

    if (queue_append((queue_t **) &TaskQueue, (queue_t *) TaskCurr) < 0)
        return ;

    getcontext(&TaskCurr->context) ;
    TaskCurr->id = idCounter++ ;
    TaskCurr->status = PPOS_RODANDO ;

    task_init(TaskDispatcher, dispatcher, NULL) ;
#if DEBUG
    queue_print("Tasks", (queue_t *) TaskQueue, print_task) ;
#endif
    // Parea já tirar o dispatcher da fila de prontos
    task_switch(TaskDispatcher) ;
    
    return ;
}

int task_init (task_t *task,			// descritor da nova tarefa
               void  (*start_func)(void *),	// funcao corpo da tarefa
               void   *arg) {			// argumentos para a tarefa
    char *stack ;

    if (task == NULL)
        return PPOS_TASK_ERROR_CODE ;

    stack = calloc(PPOS_STACK_SZ, 1) ;
    if (stack == NULL)
        return PPOS_TASK_ERROR_CODE ;

    getcontext(&task->context) ;
    task->context.uc_stack.ss_sp = stack ;
    task->context.uc_stack.ss_size = PPOS_STACK_SZ ;
    task->context.uc_stack.ss_flags = 0 ;
    task->context.uc_link = 0 ;
    makecontext(&task->context, (void (*)(void))start_func, 1, arg) ;

    task->id = idCounter++ ;

    if (queue_append((queue_t **) &TaskQueue, (queue_t *) task) < 0)
        return PPOS_TASK_ERROR_CODE ;

    task->status = PPOS_PRONTA ;

#ifdef DEBUG
    PPOS_DEBUG("Inicializando task %d. (func = %p)", task->id, start_func) ;
#endif
     return task->id ;
}

int task_id () {
    return TaskCurr->id ;
}

void task_exit (int exit_code) {
    (void)exit_code ;
#ifdef DEBUG
    PPOS_DEBUG("Saindo task %d.", task_id()) ;
#endif
    if (TaskCurr == TaskDispatcher) {
#ifdef DEBUG
        PPOS_DEBUG("%s", "Saindo Main.") ;
#endif
        free(TaskCurr->context.uc_stack.ss_sp) ;
        exit(exit_code) ;
    }
#ifdef DEBUG
    PPOS_DEBUG("%s", "Voltando para task dispatcher") ;
#endif
    TaskCurr->status = PPOS_TERMINADA ;
    task_switch(TaskDispatcher) ;
}

int task_switch (task_t *task) {
    task_t *aux ;
    if (task == NULL)
        return PPOS_TASK_ERROR_CODE ;

    if (task == TaskCurr)
        return PPOS_TASK_OK_CODE ;

    aux = TaskCurr ;
#ifdef DEBUG
    PPOS_DEBUG("Trocando tasks %d -> %d", task_id(), task->id) ;
#endif

    TaskCurr = task ;
    swapcontext(&aux->context, &task->context) ;

    return PPOS_TASK_OK_CODE ;
}

void task_yield () {
    TaskCurr->status = PPOS_PRONTA ;
    TaskQueue = TaskQueue->next ;

#ifdef DEBUG
    PPOS_DEBUG("Yield da task %d, indo para dispatcher (%d)", task_id(), TaskDispatcher->id) ;
#endif
    task_switch(TaskDispatcher) ;
}

static task_t *scheduler(void) {
    return TaskQueue ;
}

static void dispatcher(void * arg) {
    (void)arg ;
    task_t *proxima ;
#ifdef DEBUG
    PPOS_DEBUG("%s", "Iniciando dispatcher") ;
#endif
    if (queue_remove((queue_t **) &TaskQueue, (queue_t *) TaskDispatcher) < 0) {
        fprintf(stderr, "[UNREACHABLE]: Mão deveria chegar aqui\n") ;
        exit(1) ;
    }

    while (queue_size((queue_t *) TaskQueue)) {
        proxima = scheduler() ;
        if (proxima != NULL) {
            proxima->status = PPOS_RODANDO ;
#ifdef DEBUG
    PPOS_DEBUG("Proxima task %d", proxima->id) ;
#endif
            task_switch(proxima) ;
            
            switch (proxima->status) {
                case PPOS_TERMINADA:
                    if (queue_remove((queue_t **) &TaskQueue, (queue_t *) proxima) < 0)
                        exit(1);
                    if (proxima != &__TaskMain)
                        free(proxima->context.uc_stack.ss_sp) ;
                    break ;
                case PPOS_PRONTA:
                    break ;
                default:
                    fprintf(stderr, "[UNREACHABLE]: Não deveria chegar aqui\n") ;
                    exit(1) ;
                    break ; 
            }
        }
    }

    task_exit(0) ;
}
