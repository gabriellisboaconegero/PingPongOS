# Relatório de trabalho
Nome: Gabriel Lisboa Conegero
GRR: 20221255

# Como compilar
É preciso ter os seguintes arquivos .h
- ppos.h
- ppos_core.h
- ppos_kernel_funcs.h
```console
gcc -Wall -Wextra -c ppos_core.c -o ppos_core.o
gcc -Wall -Wextra -c queue.c -o queue.o
gcc -Wall -Wextra -c ppos_ipc.c -o ppos_ipc.o
gcc -Wall -Wextra -c ppos_mqueue.c -o ppos_mqueue.o
gcc -Wall -Wextra <arquivo.c> -o <arquivo.o>
gcc ping-pong-mqueue.o ppos_core.o queue.o ppos_ipc.o <arquivo.o> -o <executavel> -lm
```
