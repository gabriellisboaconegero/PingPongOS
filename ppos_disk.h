// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__
#include "ppos.h"

#define PPOS_DISK_READ_OP   0
#define PPOS_DISK_WRITE_OP  1
#define PPOS_DISK_READY     2
#define PPOS_DISK_MGR_ERROR -1

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

// estrutura que representa um disco no sistema operacional
typedef struct
{
    // completar com os campos necessarios
    short initialized ;             // Se gerente de disco foi inicializado
    semaphore_t s_disk ;            // Semaforo de acesso ao disco
    semaphore_t s_pronto ;          // Semaforo de pedidos requisitados ao disco
    int blk_size ;                  // Tamanho do bloco do disco
    int num_blks ;                  // Quantidade de blocos
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
