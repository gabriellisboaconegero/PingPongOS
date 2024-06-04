#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "ppos.h"
#include "ppos_data.h"

// Alocação global das tarefas mais e dispatcher
task_t __TaskDispatcher ;
task_t __TaskMain ;
// Ponteiros para as alocações globais
task_t *TaskCurr = &__TaskMain ;
task_t *TaskDispatcher = &__TaskDispatcher ;

// Fila de tarefas prontas
task_t *ReadyQueue ;
// Fila de tarefas dormindo
static task_t *SleepQueue ;
// Tarefa com menor tempo para acordar
static task_t *MinTaskAwake ;

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
// Definie funções wrappers e de kernel
// Vem aqui para poder acessar TaskCurr, etc.
#define PPOS_KERNEL_FUNCS_IMPL
#define PPOS_KERNEL_BUSY_CS
#ifdef DEBUG_LOCK
    #undef PPOS_KERNEL_BUSY_CS
    /* #warning Fazendo com Kernel Locks */
#else
    /* #warning Fazendo com Busy Cs */
#endif
#include "ppos_kernel_funcs.h"
#undef PPOS_KERNEL_BUSY_CS
#undef PPOS_KERNEL_FUNCS_IMPL

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

static void print_task_awake_time (void *ptr) {
   task_t *elem = ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d (%d)>", elem->id, elem->awake_t) ;
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
    // Se a tarefa for de sistema ou estiver executando uma kernel function
    if (TaskCurr->is_sys || TaskCurr->is_sys_func > 0)
        return ;
       
    TaskCurr->quanta-- ;

#if defined(DEBUG) && defined(TICK_DEBUG)
    PPOS_DEBUG("Task %d quanta [%d]", TaskCurr->id, TaskCurr->quanta) ;
#endif
    if (TaskCurr->quanta <= 0){
        kernel_task_yield() ;
    }
}
// =============== Funções de preempção  ===============

// =============== Funções de Medida de tempo  ===============
inline unsigned int systime () {
    return ppos_time ;
}
// =============== Funções de Medida de tempo  ===============

// =============== Funções gerais ===============
void ppos_init () {
#ifdef DEBUG
    PPOS_DEBUG("%s", "Inicializando PpOS\n") ;
#endif
    setvbuf (stdout, 0, _IONBF, 0) ;

    // Coloca main na fila de prontas
    if (queue_append((queue_t **) &ReadyQueue, (queue_t *) TaskCurr) < 0)
        return ;

    // Configura a task main
    getcontext(&TaskCurr->context) ;
    TaskCurr->id = idCounter++ ;
    TaskCurr->status = PPOS_RODANDO ;
    kernel_task_setprio(NULL, PPOS_DEFAULT_PRIO) ;
    TaskCurr->is_sys = 0 ;
    TaskCurr->is_sys_func = 0 ;
    // Igual a TaskCurr->is_sys_func = 1 ;
    kernel_lock() ;

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
    kernel_task_init(TaskDispatcher, dispatcher, NULL) ;
    TaskDispatcher->is_sys = 1 ;
#ifdef DEBUG
    queue_print("Tasks", (queue_t *) ReadyQueue, print_task) ;
#endif
    // Arma temporizador logo antes de ir para dispatcher
    if (setitimer(ITIMER_REAL, &tick_timer, 0) < 0) {
        perror("PPOS[ERROR]: Erro em setitimer:");
        exit(1) ;
    }

    // Para já tirar o dispatcher da fila de prontos
    kernel_task_switch(TaskDispatcher) ;
    
    kernel_unlock() ;
    return ;
}

static void print_task_sum() {
    printf("Task %d exit: execution time %6d ms, processor %6d ms, %4d activations\n",
                                            TaskCurr->id, TaskCurr->t_end - TaskCurr->t_ini,
                                            TaskCurr->t_proc, TaskCurr->actvs) ;
}
// =============== Funções gerais ===============

// =============== Gerência de tarefas ===============
int kernel_task_init (task_t *task,			// descritor da nova tarefa
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
    kernel_task_setprio(task, PPOS_DEFAULT_PRIO) ;
    task->is_sys = 0 ;
    task->t_ini = systime() ;
    task->t_proc = 0 ;
    task->actvs = 0 ;

    // Coloca tarefa na fila de prontas
    if (queue_append((queue_t **) &ReadyQueue, (queue_t *) task) < 0)
        return PPOS_TASK_ERROR_CODE ;

    task->status = PPOS_PRONTA ;

#ifdef DEBUG
    PPOS_DEBUG("Inicializando task %d. (func = %p)", task->id, start_func) ;
#endif
    return task->id ;
}

inline int task_id () {
    return TaskCurr->id ;
}

void kernel_task_exit(int exit_code) {
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
    TaskCurr->ret_cod = exit_code ;
    kernel_task_switch(TaskDispatcher) ;
    fprintf(stderr, "[UNREACHABLE][%s]: Não deveria chegar aqui, função ja terminada\n", __func__) ;
}

int kernel_task_switch (task_t *task) {
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

void kernel_task_yield () {
    TaskCurr->status = PPOS_PRONTA ;

#ifdef DEBUG
    PPOS_DEBUG("Yield da task %d, indo para dispatcher (%d)", task_id(), TaskDispatcher->id) ;
#endif
    kernel_task_switch(TaskDispatcher) ;
}

void kernel_task_suspend (task_t **queue) {
#ifdef DEBUG
    PPOS_DEBUG("Task %d suspensa (%d)", task_id(), systime()) ;
#endif
    // Se task->prev não for NULL quer dizer que está em uma fila, 
    // Se for NULL quer dizer que não está em uma fila
    if (TaskCurr->prev != NULL && queue_remove((queue_t **) &ReadyQueue, (queue_t *) TaskCurr) < 0)
        exit(1) ;
    TaskCurr->status = PPOS_SUSPENSA ;
    if (queue_append((queue_t **) queue, (queue_t *) TaskCurr) < 0)
        exit(1) ;
    kernel_task_switch(TaskDispatcher) ;
}

void kernel_task_awake (task_t *task, task_t **queue) {
#ifdef DEBUG
    PPOS_DEBUG("Task %d acordada (%d)", task->id, systime()) ;
#endif
    if (queue_remove((queue_t **) queue, (queue_t *) task) < 0)
        exit(1) ;
    TaskCurr->status = PPOS_PRONTA ;
    if (queue_append((queue_t **) &ReadyQueue, (queue_t *) task) < 0)
        exit(1) ;
}

int kernel_task_wait (task_t *task) {
#ifdef DEBUG
    PPOS_DEBUG("Task %d esperando por Task %d", task_id(), task->id) ;
#endif
    if (task == NULL)
        return PPOS_TASK_ERROR_CODE ;
    if (task->status != PPOS_TERMINADA)
        kernel_task_suspend(&task->wait_queue) ;
    return task->ret_cod ;
}

void kernel_task_sleep (int t) {
    // Não colocar o dispatcher na fila de dorminhocas
    if (TaskCurr == TaskDispatcher)
        return ;
    TaskCurr->awake_t = systime() + t ;
    if (MinTaskAwake == NULL || TaskCurr->awake_t < MinTaskAwake->awake_t)
        MinTaskAwake = TaskCurr ;
    kernel_task_suspend(&SleepQueue) ;
}

// =============== Gerência de tarefas ===============

// =============== Funções de escalonamento  ===============
void kernel_task_setprio (task_t *task, int prio) {
    if (task == NULL){
        TaskCurr->prio = CLAMP_PRIO(prio) ;
        TaskCurr->dprio = TaskCurr->prio;
        return ;
    }

    task->prio = CLAMP_PRIO(prio) ;
    task->dprio = task->prio;
}

int kernel_task_getprio (task_t *task) {
    if (task == NULL)
        return TaskCurr->prio ;
    return task->prio ;
}

static task_t *scheduler(void) {
    task_t *it = ReadyQueue ;
    task_t *min_prio = it ;

    // Se não tiver mais tasks para ser executadas
    if (it == NULL)
        return ReadyQueue ;

    it = it->next ;
    while (it != ReadyQueue){
        if (it->dprio < min_prio->dprio){
            min_prio->dprio = CLAMP_PRIO(min_prio->dprio + PPOS_PRIO_DELTA) ;
            min_prio = it ;
        } else {
            it->dprio = CLAMP_PRIO(it->dprio + PPOS_PRIO_DELTA) ;
        }
         it = it->next ;
    }
#ifdef DEBUG
    printf("MIN_DPRIO: %d (%d)\n", min_prio->id, min_prio->dprio) ;
    queue_print("Tasks Prio", (queue_t *) ReadyQueue, print_task_prio) ;
#endif
    if (queue_remove((queue_t **)&ReadyQueue, (queue_t *)min_prio) < 0)
        exit(1) ;

    return min_prio ;
}

static void awake_task_queue(task_t *task) {
    if (task == NULL || task->wait_queue == NULL)
        return ;
    
    while (task->wait_queue != NULL)
        kernel_task_awake(task->wait_queue, &task->wait_queue) ;
}

static void check_sleep_queue() {
    if (!queue_size((queue_t *) SleepQueue))
        return ;
    // Não verificando MinTaskAwake == NULL pois é redundante
    // Se não tiver atrefa dormindo então MinTaskAwake deve ser NULL
    // Se tiver errado a implementação é melhor que de segfault
    // Ajuda a pegar o erro.
    if (systime() < MinTaskAwake->awake_t)
        return ;
    MinTaskAwake = NULL ;

    // Começa iteração pela segunda task da lista
    task_t *it = SleepQueue->next ;
    // Itera até voltar para primeira task
    while(it != SleepQueue){
        // Verifica se a task deve se acordada
        if (it->awake_t <= systime()){
            // Atualiza it para a próxima iteração
            it = it->next ;
            // Acorda task
            kernel_task_awake(it->prev, &SleepQueue) ;
        }else{
            // Verifica se é a task de menor tempo
            if (MinTaskAwake == NULL || it->awake_t < MinTaskAwake->awake_t)
                MinTaskAwake = it ;
            it = it->next ;
        }
    }

    // Verifica se a  primeira task deve ser retirada
    if (it->awake_t <= systime())
        kernel_task_awake(it, &SleepQueue) ;
    else if (MinTaskAwake == NULL || it->awake_t < MinTaskAwake->awake_t)
        MinTaskAwake = it ;
#ifdef DEBUG
    if (MinTaskAwake != NULL)
        printf("MIN_AWAKE_TASK: %d (%d)\n", MinTaskAwake->id, MinTaskAwake->awake_t) ;
    queue_print("Tasks Awake Time", (queue_t *) SleepQueue, print_task_awake_time) ;
#endif
}

static void dispatcher(void * arg) {
    (void)arg ;
    task_t *proxima ;
#ifdef DEBUG
    PPOS_DEBUG("%s", "Iniciando dispatcher") ;
#endif
    // Remove o dispatcher da fila de prontos
    if (queue_remove((queue_t **) &ReadyQueue, (queue_t *) TaskDispatcher) < 0) {
        fprintf(stderr, "[UNREACHABLE]: Não deveria chegar aqui\n") ;
        exit(1) ;
    }

    // Enquanto tiver tasks para executar na fila de prontos
    while (queue_size((queue_t *) ReadyQueue) || queue_size((queue_t *) SleepQueue)) {
        check_sleep_queue() ;
        proxima = scheduler() ;
        if (proxima != NULL) {
            proxima->status = PPOS_RODANDO ;
#ifdef DEBUG
    PPOS_DEBUG("Proxima task %d", proxima->id) ;
#endif
            proxima->quanta = PPOS_QUANTA ;
            // Executa task proxima
            kernel_task_switch(proxima) ;
            
            switch (proxima->status) {
                case PPOS_TERMINADA:
                    /* if (queue_remove((queue_t **) &ReadyQueue, (queue_t *) proxima) < 0) */
                    /*     exit(1); */
                    awake_task_queue(proxima) ;
                    if (proxima != &__TaskMain)
                        free(proxima->context.uc_stack.ss_sp) ;
                    break ;
                case PPOS_PRONTA:
                    // Coloca na fila de prontas de volta
                    if (queue_append((queue_t **) &ReadyQueue, (queue_t *) proxima) < 0)
                        exit(1) ;
                    // Tarefa executada, restaura sua prioridade fixa
                    proxima->dprio = proxima->prio ;
                    break ;
                case PPOS_SUSPENSA: break;
                default:
                    fprintf(stderr, "[UNREACHABLE]: Não deveria chegar aqui\n") ;
                    exit(1) ;
                    break ; 
            }
        }
    }

    kernel_task_exit(0) ;
}
// =============== Funções de escalonamento  ===============
