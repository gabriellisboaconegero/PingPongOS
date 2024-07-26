// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__
#include "ppos.h"

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

// estrutura que representa um disco no sistema operacional
typedef struct
{
    // completar com os campos necessarios
    short initialized ;     // Se gerente de disco foi inicializado
    semaphore_t s_disk ;    // Semaforo de acesso ao disco
    int blk_size ;          // Tamanho do bloco do disco
    int num_blks ;          // Quantidade de blocos
    struct pedido_t {
        struct pedido_t *prev, *next ;
        task_t *request_task ;  // Tarefa que fez pedido
        void *buffer ;          // Buffer de dados que task passou
        int block ;             // Bloco da operação
        int type ;              // Tipo do pedido (0 - leitura, 1 - escrita)
    } *fila_disco ;         // Fila de pedidos
    task_t *waiting_queue ; // Fila de espera
} disk_t ;

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

#endif
