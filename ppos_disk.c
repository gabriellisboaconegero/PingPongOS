// Gabriel Lisboa Conegero - GRR20221255
#include "ppos.h"
#include "ppos_disk.h"
#include "disk.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

static disk_t Disk ;
static struct sigaction disk_sig_action ;

static void disk_sig_handler(int signum){
    (void)signum ;
    if (disk_cmd(DISK_CMD_STATUS, 0, 0) == 1){
        sem_up(&Disk.s_pronto) ;
    }
}

int disk_mgr_init (int *numBlocks, int *blockSize) {
    if (sem_init(&Disk.s_disk, 1) < 0)
        return PPOS_DISK_MGR_ERROR ;

    if (sem_init(&Disk.s_pronto, 0) < 0)
        return PPOS_DISK_MGR_ERROR ;

    Disk.initialized = disk_cmd(DISK_CMD_INIT, 0, 0) >= 0 ;
    if (!Disk.initialized)
        return PPOS_DISK_MGR_ERROR ;

    Disk.blk_size = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0) ;
    if (Disk.blk_size < 0)
        return PPOS_DISK_MGR_ERROR ;

    Disk.num_blks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0) ;
    if (Disk.num_blks < 0)
        return PPOS_DISK_MGR_ERROR ;

    disk_sig_action.sa_handler = disk_sig_handler ;
    sigemptyset(&disk_sig_action.sa_mask) ;
    disk_sig_action.sa_flags = 0 ;
    if (sigaction(SIGUSR1, &disk_sig_action, 0) < 0)
        return PPOS_DISK_MGR_ERROR ;

    /* if (task_init(TaskDiskMgr, disk_driver, NULL) < 0) */
    /*     return PPOS_DISK_MGR_ERROR ; */

    *numBlocks = Disk.num_blks ;
    *blockSize = Disk.blk_size ;
    return 0 ;
}
int disk_block_read (int block, void *buffer) {
    if (buffer == NULL || !Disk.initialized || block >= Disk.num_blks)
        return PPOS_DISK_MGR_ERROR ;
    if (sem_down(&Disk.s_disk) < 0)
        return PPOS_DISK_MGR_ERROR ;
    if (disk_cmd(DISK_CMD_READ, block, buffer) < 0){
        sem_up(&Disk.s_disk) ;
        return PPOS_DISK_MGR_ERROR ;
    }
    sem_down(&Disk.s_pronto) ;
    sem_up(&Disk.s_disk) ;
    return 0;
}

int disk_block_write (int block, void *buffer) {
    if (buffer == NULL || !Disk.initialized || block >= Disk.num_blks)
        return PPOS_DISK_MGR_ERROR ;
    if (sem_down(&Disk.s_disk) <  0)
        return PPOS_DISK_MGR_ERROR ;
    if (disk_cmd(DISK_CMD_WRITE,  block, buffer) < 0){
        sem_up(&Disk.s_disk) ;
        return PPOS_DISK_MGR_ERROR ;
    }
    sem_down(&Disk.s_pronto) ;
    sem_up(&Disk.s_disk) ;
    return 0;

}
