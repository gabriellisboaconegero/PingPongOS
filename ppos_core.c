#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

#define PPOS_TASK_ERROR_CODE -1
#define PPOS_TASK_OK_CODE 0
#define PPOS_STACK_SZ 1<<14
#define PPOS_DEBUG(msg, ...) printf("PPOS[%s]: "msg"\n", __func__, __VA_ARGS__)

task_t __TaskMain ;
task_t *TaskCurr ;
task_t *TaskMain = &__TaskMain ;
int idCounter = 0 ;

void ppos_init () {
#if DEBUG
    PPOS_DEBUG("%s", "Inicializando PpOS\n") ;
#endif
    TaskCurr = TaskMain ;
    setvbuf (stdout, 0, _IONBF, 0) ;
    getcontext(&TaskCurr->context) ;
    TaskCurr->id = idCounter++ ;
    
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
    if (TaskCurr == TaskMain){
#ifdef DEBUG
        PPOS_DEBUG("%s", "Saindo Main.") ;
#endif
        exit(exit_code) ;
    }
#ifdef DEBUG
    PPOS_DEBUG("%s", "Voltando para task main.") ;
#endif
    //free(TaskCurr->context.uc_stack.ss_sp) ;
    task_switch(TaskMain) ;
}

int task_switch (task_t *task) {
    task_t *aux ;
    if (task == NULL)
        return PPOS_TASK_ERROR_CODE ;

    aux = TaskCurr ;
#ifdef DEBUG
    PPOS_DEBUG("Trocando tasks %d -> %d", task_id(), task->id) ;
#endif

    TaskCurr = task ;
    swapcontext(&aux->context, &task->context) ;
    TaskCurr = aux ;

    return PPOS_TASK_OK_CODE ;
}
