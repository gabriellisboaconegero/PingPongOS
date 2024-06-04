
// Gabriel Lisboa Conegero - GRR20221255
// Declaração de funções internas do kernel, mesmas funções declaradas em ppos.h.
// Porém as funções em ppos.h são wrapper functions das internas do kernel (esse arquivo)
// onde elas fazem o papel de chamar a funções kernel_lock e kernel_unlock que vão definir
// que TaskCurr está  executando uma tarefa de kernel
//
// ATENÇÃO: Lembrar de adicionar toda função nova que deva ser bloqueate
//      Definir no arquivo pricipal em que as funções wrapper deve ser declaradas usando
// ATENÇÃO: #define PPOS_KERNEL_FUNCS_IMPL antes de #include "ppos_kernel_funcs.h"
//      Isso para poder utilizar o mesmo arquivo como implementação e definição de funções kernel
// ATENÇÃO: #define PPOS_KERNEL_BUSY_CS antes de #include "ppos_kernel_funcs.h"
//      Fazer isso se quiser a implementação usando busy wait com operações atômicas
//      ao invés do mecanismo de exclusão mútua TaskCurr->is_sys_func
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
#endif // PPOS_KERNEL_FUNCS_IMPL
#ifdef PPOS_KERNEL_BUSY_CS
inline static void sem_enter_cs(int *lock){
    while(__sync_fetch_and_or(lock, 1)) ;
}

inline static void sem_leave_cs(int *lock){
    *lock = 0 ;
}
#endif // PPOS_KERNEL_BUSY_CS
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
#ifdef PPOS_KERNEL_BUSY_CS
inline int sem_init (semaphore_t *s, int value) {
    if (s != NULL)
        sem_enter_cs(&s->lock) ;
    int ans = kernel_sem_init(s, value) ;
    if (s != NULL)
        sem_leave_cs(&s->lock) ;
    return ans ;
}
#else
inline int sem_init (semaphore_t *s, int value) {
    kernel_lock() ;
    int ans = kernel_sem_init(s, value) ;
    kernel_unlock() ;
    return ans ;
}
#endif // PPOS_KERNEL_FUNCS_IMPL
#endif // PPOS_KERNEL_BUSY_CS

int kernel_sem_down (semaphore_t *s) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
// ATENÇÃO: sem_down é especial, pois não da para garantir sem_leave_cs
// antes de fazer o switch, então deve ser dentro da implementação
// de kernel_sem_down o sem_enter_cs e o sem_leave_cs.
#ifdef PPOS_KERNEL_BUSY_CS
inline int sem_down (semaphore_t *s) {
    return kernel_sem_down(s) ;
}
#else
inline int sem_down (semaphore_t *s) {
    kernel_lock() ;
    int ans = kernel_sem_down(s) ;
    kernel_unlock() ;
    return ans ;
}
#endif // PPOS_KERNEL_FUNCS_IMPL
#endif // PPOS_KERNEL_BUSY_CS

int kernel_sem_up (semaphore_t *s) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
#ifdef PPOS_KERNEL_BUSY_CS
inline int sem_up (semaphore_t *s) {
    if (s != NULL)
        sem_enter_cs(&s->lock) ;
    int ans = kernel_sem_up(s) ;
    if (s != NULL)
        sem_leave_cs(&s->lock) ;
    return ans ;
}
#else
inline int sem_up (semaphore_t *s) {
    kernel_lock() ;
    int ans = kernel_sem_up(s) ;
    kernel_unlock() ;
    return ans ;
}
#endif // PPOS_KERNEL_FUNCS_IMPL
#endif // PPOS_KERNEL_BUSY_CS

int kernel_sem_destroy (semaphore_t *s) ;
#ifdef PPOS_KERNEL_FUNCS_IMPL
#ifdef PPOS_KERNEL_BUSY_CS
inline int sem_destroy (semaphore_t *s) {
    if (s != NULL)
        sem_enter_cs(&s->lock) ;
    int ans = kernel_sem_destroy(s) ;
    if (s != NULL)
        sem_leave_cs(&s->lock) ;
    return ans ;
}
#else
inline int sem_destroy (semaphore_t *s) {
    kernel_lock() ;
    int ans = kernel_sem_destroy(s) ;
    kernel_unlock() ;
    return ans ;
}
#endif // PPOS_KERNEL_FUNCS_IMPL
#endif // PPOS_KERNEL_BUSY_CS
// ================= Funções bloqueantes ===================

#endif
