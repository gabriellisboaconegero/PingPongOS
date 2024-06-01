// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;  // ponteiros para usar em filas
  int id ;                      // identificador da tarefa
  ucontext_t context ;          // contexto armazenado da tarefa
  short status ;                // pronta, rodando, suspensa, ...
  short prio, dprio ;           // Prioridade estatia e dinamicaa
  short quanta ;                // Quanta de tempo definido pelo dispatcher
  short is_sys ;                // Informa se é uma task de kernel ou não
  short is_sys_func ;           // Informa se a task está executando função de sistema
  unsigned int t_ini, t_end ;   // Tempo de inicio e fim (task_init, task_end)
  unsigned int t_proc ;         // Tempo gasto na CPU
  unsigned int actvs ;          // Quantas vezes foi colocado na CPU
  struct task_t *wait_queue ;   // Fila de tasks esperando essa task terminar
  int ret_cod ;                 // Código de retorno da tarefa
  unsigned int awake_t ;        // Tempo em que a tarefa deve ser acordada
  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
    int cont ;                  // Contador do semaforo
    task_t *task_queue ;        // Fila de tarefas esperando smaforo liberar
    short dead ;                // Falg de semaforo destruido
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
  // preencher quando necessário
} mqueue_t ;

#endif

