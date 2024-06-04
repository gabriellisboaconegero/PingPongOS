// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"

#define PPOS_DEBUG(msg, ...) printf("PPOS[%s]: "msg"\n", __func__, __VA_ARGS__)

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define CLAMP(v, M, m) (MAX(MIN((v), (M)), m))

#define PPOS_TASK_ERROR_CODE -1
#define PPOS_TASK_OK_CODE 0
#define PPOS_STACK_SZ 1<<16

#define PPOS_RODANDO 0
#define PPOS_PRONTA 1
#define PPOS_TERMINADA 2
#define PPOS_SUSPENSA 3

#define PPOS_DEFAULT_PRIO 0
#define PPOS_MAX_PRIO 20
#define PPOS_MIN_PRIO -20
#define PPOS_PRIO_DELTA -1

#define PPOS_QUANTA 10
// EM micro segundos (1000 micro = 1 mili)
#define PPOS_TICK_DELTA 1000
#define CLAMP_PRIO(v) CLAMP((v), PPOS_MAX_PRIO, PPOS_MIN_PRIO)
// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
    struct task_t *prev, *next ;    // ponteiros para usar em filas
    int id ;                        // identificador da tarefa
    ucontext_t context ;            // contexto armazenado da tarefa
    short status ;                  // pronta, rodando, suspensa, ...
    short prio, dprio ;             // Prioridade estatia e dinamicaa
    short quanta ;                  // Quanta de tempo definido pelo dispatcher
    short is_sys ;                  // Informa se é uma task de kernel ou não
    short is_sys_func ;             // Informa se a task está executando função de sistema
    unsigned int t_ini, t_end ;     // Tempo de inicio e fim (task_init, task_end)
    unsigned int t_proc ;           // Tempo gasto na CPU
    unsigned int actvs ;            // Quantas vezes foi colocado na CPU
    struct task_t *wait_queue ;     // Fila de tasks esperando essa task terminar
    int ret_cod ;                   // Código de retorno da tarefa
    unsigned int awake_t ;          // Tempo em que a tarefa deve ser acordada
                                    // ... (outros campos serão adicionados mais tarde)
} task_t ;

#define PPOS_SEM_ERROR -1
#define PPOS_SEM_OK 0
// estrutura que define um semáforo
typedef struct
{
    int cont ;                      // Contador do semaforo
    task_t *task_queue ;            // Fila de tarefas esperando smaforo liberar
    short dead ;                    // Falg de semaforo destruido
    int lock ;                      // Indica se o semaforo está em seção critica
                                    // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
    int max_msgs ;                  // Número maximo de mensagens na fila
    int msg_size ;                  // Tamanho fixo de cada mensagem
    struct buffer_t {
        struct buffet_t *prev, *next ;
        void *msg ;
    } *queue ;                      // Fila de mensagens
    short dead ;                    // Se a fila foi destruida
    semaphore_t s_vaga ;            // Semaforo de controle de vagas livres
    semaphore_t s_buffer ;          // Semaforo do buffer
    semaphore_t s_item ;            // Semaforo de controle de itens prontos
    // preencher quando necessário
} mqueue_t ;

#endif

