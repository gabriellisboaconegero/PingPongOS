#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "ppos.h"

#define PPOS_TASK_ERROR_CODE -1
#define PPOS_TASK_OK_CODE 0
#define PPOS_STACK_SZ 1<<16
#define PPOS_DEBUG(msg, ...) printf("PPOS[%s]: "msg"\n", __func__, __VA_ARGS__)

#define PPOS_RODANDO 0
#define PPOS_PRONTA 1
#define PPOS_TERMINADA 2

#define PPOS_DEFAULT_PRIO 0
#define PPOS_MAX_PRIO 20
#define PPOS_MIN_PRIO -20
#define PPOS_PRIO_DELTA -1

#define PPOS_QUANTA 10
// EM micro segundos (1000 micro = 1 mili)
#define PPOS_TICK_DELTA 1000

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define CLAMP(v) (MAX(MIN((v), PPOS_MAX_PRIO), PPOS_MIN_PRIO))

static task_t __TaskDispatcher ;
static task_t __TaskMain ;
static task_t *TaskCurr = &__TaskMain ;
static task_t *TaskQueue ;
static task_t *TaskDispatcher = &__TaskDispatcher ;

// ============= Global vars =============
// Contador de id's das tasks, diz qual o id da próxima task a ser criada
static int idCounter = 0 ;

static struct sigaction tick_action ;

static struct itimerval tick_timer ;

static unsigned int ppos_time = 0 ;

static unsigned int task_curr_time = 0 ;
// ============= Global vars =============


// =============== Funções de escalonamento  ===============
static void dispatcher(void * arg) ;
// =============== Funções de escalonamento  ===============

#ifdef DEBUG
static void print_task (void *ptr) {
   task_t *elem = ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d>", elem->id) ;
   elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
}

static void print_task_prio (void *ptr) {
   task_t *elem = ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d (%d)>", elem->id, elem->dprio) ;
   elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
}

#endif

// =============== Funções de preempção  ===============
static void tick_handler(int signum) {
    (void)signum ;
#if defined(DEBUG) && defined(TICK_DEBUG)
    PPOS_DEBUG("Tick handler ativado durante execução da task %d", TaskCurr->id) ;
#endif
    // A cada tick incremeta o número de milisegundos passados
    ppos_time += PPOS_TICK_DELTA/1000 ;
    if (TaskCurr->is_sys)
        return ;
       
    TaskCurr->quanta-- ;

#if defined(DEBUG) && defined(TICK_DEBUG)
    PPOS_DEBUG("Task %d quanta [%d]", TaskCurr->id, TaskCurr->quanta) ;
#endif
    if (TaskCurr->quanta <= 0){
        task_yield() ;
    }
}
// =============== Funções de preempção  ===============

// =============== Funções de Medida de tempo  ===============
unsigned int systime () {
    return ppos_time ;
}
// =============== Funções de Medida de tempo  ===============

// =============== Funções gerais ===============
void ppos_init () {
#if DEBUG
    PPOS_DEBUG("%s", "Inicializando PpOS\n") ;
#endif
    setvbuf (stdout, 0, _IONBF, 0) ;

    // Coloca main na fila de prontas
    if (queue_append((queue_t **) &TaskQueue, (queue_t *) TaskCurr) < 0)
        return ;

    // Configura a task main
    getcontext(&TaskCurr->context) ;
    TaskCurr->id = idCounter++ ;
    TaskCurr->status = PPOS_RODANDO ;
    task_setprio(NULL, PPOS_DEFAULT_PRIO) ;
    TaskCurr->is_sys = 0 ;
    TaskCurr->t_ini = systime() ;
    TaskCurr->t_proc = 0 ;
    TaskCurr->actvs = 0 ;

    // Cria tratador de ticks, usando UNIX signals
    tick_action.sa_handler = tick_handler ;
    sigemptyset(&tick_action.sa_mask) ;
    tick_action.sa_flags = 0 ;
    if (sigaction(SIGALRM, &tick_action, 0) < 0) {
        perror("PPOS[ERROR]: Erro em sigaction: ") ;
        exit(1) ;
    }

    // Criaa timer de ticks
    tick_timer.it_value.tv_usec = PPOS_TICK_DELTA ;      // primeiro disparo, em micro-segundos
    tick_timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    tick_timer.it_interval.tv_usec = PPOS_TICK_DELTA ;   // disparos subsequentes, em micro-segundos
    tick_timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

    // Inicia dispatcher
    task_init(TaskDispatcher, dispatcher, NULL) ;
    TaskDispatcher->is_sys = 1 ;
#if DEBUG
    queue_print("Tasks", (queue_t *) TaskQueue, print_task) ;
#endif
    // Arma temporizador logo antes de ir para dispatcher
    if (setitimer(ITIMER_REAL, &tick_timer, 0) < 0) {
        perror("PPOS[ERROR]: Erro em setitimer:");
    }

    // Para já tirar o dispatcher da fila de prontos
    task_switch(TaskDispatcher) ;
    
    return ;
}

static void print_task_sum() {
    printf("Task %d exit: execution time %6d ms, processor %6d ms, %4d activations\n",
                                            TaskCurr->id, TaskCurr->t_end - TaskCurr->t_ini,
                                            TaskCurr->t_proc, TaskCurr->actvs) ;
}
// =============== Funções gerais ===============

// =============== Gerência de tarefas ===============
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
    task_setprio(task, PPOS_DEFAULT_PRIO) ;
    task->is_sys = 0 ;
    task->t_ini = systime() ;
    task->t_proc = 0 ;
    task->actvs = 0 ;

    // Coloca tarefa na fila de prontas
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
    TaskCurr->t_end = systime() ;
    // Dispatcher é a última tarefa a terminar
    if (TaskCurr == TaskDispatcher) {
        TaskCurr->t_proc += systime() - task_curr_time ;
#ifdef DEBUG
        PPOS_DEBUG("%s", "Saindo Main.") ;
#endif
        print_task_sum() ;
        free(TaskCurr->context.uc_stack.ss_sp) ;
        exit(exit_code) ;
    }
#ifdef DEBUG
    PPOS_DEBUG("%s", "Voltando para task dispatcher") ;
#endif
    print_task_sum() ;
    TaskCurr->status = PPOS_TERMINADA ;
    task_switch(TaskDispatcher) ;
}

int task_switch (task_t *task) {
    task_t *aux ;
    if (task == NULL)
        return PPOS_TASK_ERROR_CODE ;

    if (task == TaskCurr)
        return PPOS_TASK_OK_CODE ;

    // TaskCurr -> task
    // Contando tempo de proc de TaskCurr
    TaskCurr->t_proc += systime() - task_curr_time ;
    // Incrementando task->actvs
    task->actvs++ ;
    // Reseta tempo de execução da TaskCurr
    task_curr_time = systime() ;

    // Salva task atual para trocar dela para task_t *task
    aux = TaskCurr ;
#ifdef DEBUG
    PPOS_DEBUG("Trocando tasks %d -> %d", task_id(), task->id) ;
#endif

    // Muda task atual para a task_t *task
    TaskCurr = task ;
    swapcontext(&aux->context, &task->context) ;

    return PPOS_TASK_OK_CODE ;
}
// =============== Gerência de tarefas ===============

// =============== Funções de escalonamento  ===============
void task_yield () {
    TaskCurr->status = PPOS_PRONTA ;

#ifdef DEBUG
    PPOS_DEBUG("Yield da task %d, indo para dispatcher (%d)", task_id(), TaskDispatcher->id) ;
#endif
    task_switch(TaskDispatcher) ;
}

void task_setprio (task_t *task, int prio) {
    if (task == NULL){
        TaskCurr->prio = CLAMP(prio) ;
        TaskCurr->dprio = TaskCurr->prio;
        return ;
    }

    task->prio = CLAMP(prio) ;
    task->dprio = task->prio;
}

int task_getprio (task_t *task) {
    if (task == NULL)
        return TaskCurr->prio ;
    return task->prio ;
}

static task_t *scheduler(void) {
    task_t *it = TaskQueue ;
    task_t *min_prio = it ;

    // Se não tiver mais tasks para ser executadas
    if (it == NULL)
        return TaskQueue ;

    it = it->next ;
    while (it != TaskQueue){
        if (it->dprio < min_prio->dprio){
            min_prio->dprio = CLAMP(min_prio->dprio + PPOS_PRIO_DELTA) ;
            min_prio = it ;
        } else {
            it->dprio = CLAMP(it->dprio + PPOS_PRIO_DELTA) ;
        }
         it = it->next ;
    }
#ifdef DEBUG
    printf("MIN_DPRIO: %d (%d)\n", min_prio->id, min_prio->dprio) ;
    queue_print("Tasks Prio", (queue_t *) TaskQueue, print_task_prio) ;
#endif
    if (queue_remove((queue_t **)&TaskQueue, (queue_t *)min_prio) < 0)
        exit(1) ;

    return min_prio ;
}

static void dispatcher(void * arg) {
    (void)arg ;
    task_t *proxima ;
#ifdef DEBUG
    PPOS_DEBUG("%s", "Iniciando dispatcher") ;
#endif
    // Remove o dispatcher da fila de prontos
    if (queue_remove((queue_t **) &TaskQueue, (queue_t *) TaskDispatcher) < 0) {
        fprintf(stderr, "[UNREACHABLE]: Não deveria chegar aqui\n") ;
        exit(1) ;
    }

    // Enquanto tiver tasks para executar na fila de prontos
    while (queue_size((queue_t *) TaskQueue)) {
        proxima = scheduler() ;
        if (proxima != NULL) {
            proxima->status = PPOS_RODANDO ;
#ifdef DEBUG
    PPOS_DEBUG("Proxima task %d", proxima->id) ;
#endif
            proxima->quanta = PPOS_QUANTA ;
            // Executa task proxima
            task_switch(proxima) ;
            
            switch (proxima->status) {
                case PPOS_TERMINADA:
                    /* if (queue_remove((queue_t **) &TaskQueue, (queue_t *) proxima) < 0) */
                    /*     exit(1); */
                    if (proxima != &__TaskMain)
                        free(proxima->context.uc_stack.ss_sp) ;
                    break ;
                case PPOS_PRONTA:
                    // Coloca na fila de prontas de volta
                    if (queue_append((queue_t **) &TaskQueue, (queue_t *) proxima) < 0)
                        exit(1) ;
                    // Tarefa executada, restaura sua prioridade fixa
                    proxima->dprio = proxima->prio ;
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
// =============== Funções de escalonamento  ===============
