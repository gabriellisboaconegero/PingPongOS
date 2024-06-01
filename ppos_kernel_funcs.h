
// Gabriel Lisboa Conegero - GRR20221255
// Declaração de funções internas do kernel, mesmas funções declaradas em ppos.h.
// Porém as funções em ppos.h são wrapper functions das internas do kernel (esse arquivo)
// onde elas fazem o papel de chamar a funções kernel_lock e kernel_unlock que vão definir
// que TaskCurr está  executando uma tarefa de kernel
//
// ATENÇÃO: Lembrar de adicionar toda função nova que deva ser bloqueate
// Definir no arquivo pricipal em que as funções wrapper deve ser declaradas usando
// #define PPOS_KERNEL_FUNCS_IMPL antes de #include "ppos_kernel_funcs.h"
// Isso para poder utilizar o mesmo arquivo como implementação e definição de funções kernel
#ifndef __PPOS_KERNEL_FUNCS_H__
#define __PPOS_KERNEL_FUNCS_H__
#include "ppos.h"
#ifdef PPOS_KERNEL_FUNCS_IMPL
// Bloquea interrupções na task atual
inline static void kernel_lock(){
    TaskCurr->is_sys_func = 1 ;
}

// Libera interrupções na task atual
inline static void kernel_unlock(){
    TaskCurr->is_sys_func = 0 ;
}
#endif
// ================= Funções bloqueantes ===================

int kernel_task_init (task_t *task, void  (*start_func)(void *), void   *arg) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline int task_init (task_t *task, void (*start_func)(void *), void *arg) {
    kernel_lock() ;
    int ans = kernel_task_init(task, start_func, arg) ;
    kernel_unlock() ;
    return ans ;
}
#endif

void kernel_task_exit (int exit_code) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline void task_exit (int exit_code) {
    kernel_lock() ;
    kernel_task_exit(exit_code) ;
    kernel_unlock() ;
}
#endif

int kernel_task_switch (task_t *task) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline int task_switch (task_t *task) {
    kernel_lock() ;
    int ans = kernel_task_switch(task) ;
    kernel_unlock() ;
    return ans ;
}
#endif

void kernel_task_suspend (task_t **queue) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline void task_suspend (task_t **queue) {
    kernel_lock() ;
    kernel_task_suspend(queue) ;
    kernel_unlock() ;
}
#endif

void kernel_task_awake (task_t *task, task_t **queue) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline void task_awake (task_t *task, task_t **queue) {
    kernel_lock() ;
    kernel_task_awake(task, queue) ;
    kernel_unlock() ;
}
#endif

void kernel_task_yield () ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline void task_yield () {
    kernel_lock() ;
    kernel_task_yield() ;
    kernel_unlock() ;   
}
#endif

void kernel_task_setprio (task_t *task, int prio) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline void task_setprio (task_t *task, int prio) {
    kernel_lock() ;
    kernel_task_setprio(task, prio) ;
    kernel_unlock() ;
}
#endif

int kernel_task_getprio (task_t *task) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline int task_getprio (task_t *task) {
    kernel_lock() ;
    int ans = kernel_task_getprio(task) ;
    kernel_unlock() ;
    return ans ;
}
#endif

void kernel_task_sleep (int t) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline void task_sleep (int t) {
    kernel_lock() ;
    kernel_task_sleep(t) ;
    kernel_unlock() ;
}
#endif

int kernel_task_wait (task_t *task) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline int task_wait (task_t *task) {
    kernel_lock() ;
    int ans = kernel_task_wait(task) ;
    kernel_unlock() ;
    return ans ;
}
#endif

int kernel_sem_init (semaphore_t *s, int value) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline int sem_init (semaphore_t *s, int value) {
    kernel_lock() ;
    int ans = kernel_sem_init(s, value) ;
    kernel_unlock() ;
    return ans ;
}
#endif

int kernel_sem_down (semaphore_t *s) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline int sem_down (semaphore_t *s) {
    kernel_lock() ;
    int ans = kernel_sem_down(s) ;
    kernel_unlock() ;
    return ans ;
}
#endif

int kernel_sem_up (semaphore_t *s) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline int sem_up (semaphore_t *s) {
    kernel_lock() ;
    int ans = kernel_sem_up(s) ;
    kernel_unlock() ;
    return ans ;
}
#endif

int kernel_sem_destroy (semaphore_t *s) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
inline int sem_destroy (semaphore_t *s) {
    kernel_lock() ;
    int ans = kernel_sem_destroy(s) ;
    kernel_unlock() ;
    return ans ;
}
#endif
// ================= Funções bloqueantes ===================

#endif
