#include "ppos.h"
#include "ppos_disk.h"
#include "disk.h"

static disk_t Disk ;

extern task_t *TaskDispatcher ;
extern task_t *TaskDiskMgr ;
extern task_t *ReadyQueue ;

static semaphore_t s_disk_activation ;

static void disk_driver(void *args) {
    task_t *to_awake_task ;
    struct pedido_t *primeira ;
    while(Disk.initialized){
        if (sem_down(&s_disk_activation) < 0 && sem_down(&Disk.s_disk)){
            printf("[ERRO: DISK DRIVER]: Erro nos semáforos do gerente de disco\n") ;
            task_exit(1) ;
        }

        // Se disco está livre
        if (disk_cmd(DISK_CMD_STATUS, 0, 0) == 1){
            // Se disco ficou livre e tem alguém na fila de esperando disco acabar então acorda
            if (to_awake_task != NULL){
                to_awake_task = Disk.waiting_queue ;
                task_awake(to_awake_task, &Disk.waiting_queue) ;
            }
            // Se tem pedido para fazer ao disco
            if (queue_size((queue_t *) Disk.fila_disco)){
                primeira = Disk.fila_disco ;
                if (queue_remove((queue_t **) &Disk.fila_disco, (queue_t *) primeira) < 0){
                    printf("[ERRO: DISK DRIVER]: Erro ao remover pedido da fila de pedidos\n") ;
                    task_exit(1);
                }
                if (primeira->type == PPOS_DISK_READ_OP)
                    disk_cmd(DISK_CMD_READ, primeira->block, primeira->buffer) ;
                else if (primeira->type == PPOS_DISK_WRITE_OP)
                    disk_cmd(DISK_CMD_write, primeira->block, primeira->buffer) ;
            }
        }

        sem_up(&Disk.s_disk) ;
    }
}

int disk_mgr_init (int *numBlocks, int *blockSize) {
    if (sem_init(&Disk.s_disk, 1) < 0)
        return PPOS_DISK_MGR_NO_SEM ;

    Disk.initialized = disk_cmd(DISK_CMD_INIT, 0, 0) < 0 ;
    if (!Disk.initialized)
        return PPOS_DISK_MGR_DISK_INIT_ERROR ;

    Disk.blk_size = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0) ;
    if (Disk.blk_size < 0)
        return PPOS_DISK_MGR_DISK_INIT_ERROR ;

    Disk.num_blks = cd_disk(DISK_CMD_DISKSIZE, 0, 0) ;
    if (Disk.num_blks < 0)
        return PPOS_DISK_MGR_DISK_INIT_ERROR ;

    if (task_init(TaskDiskMgr, disk_driver, NULL) < 0)
        return PPOS_DISK_MGR_NO_TASK ;

    *numBlocks = Disk.num_blks ;
    *blockSize = Disk.blk_size ;
    return 0 ;
}
