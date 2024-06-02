#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ppos.h"

#define NUMFILO 5

task_t filosofo [NUMFILO] ;	// threads filosofos
semaphore_t     hashi    [NUMFILO] ;	// um semaforo para cada palito (iniciam em 1)

// espaços para separar as colunas de impressão
char *space[] = {"", "\t", "\t\t", "\t\t\t", "\t\t\t\t" } ;

// espera um tempo aleatório entre 0 e n segundos (float)
void espera (int n)
{
  return ;
  task_sleep (random() % (n * 100)) ;		// pausa entre 0 e n segundos (inteiro)
}

// filósofo comendo
void come (int f)
{
  printf ("%sF%d COMENDO\n", space[f], f) ;
  espera (1) ;
}

// filósofo meditando
void medita (int f)
{
  printf ("%sF%d meditando\n", space[f], f) ;
  espera (1) ;
}

// pega o hashi
void pega_hashi (int f, int h)
{
  printf ("%sF%d quer  h%d\n", space[f], f, h) ;
  sem_down (&hashi [h]) ;
  printf ("%sF%d pegou h%d\n", space[f], f, h) ;
}

// larga o hashi
void larga_hashi (int f, int h)
{
  printf ("%sF%d larga h%d\n", space[f], f, h) ;
  sem_up (&hashi [h]) ;
}

// corpo da thread filosofo
void threadFilosofo (void *arg)
{
  int i = (long int) arg ;
  while (1)
  {
    medita (i) ;
    pega_hashi (i, i) ;
    pega_hashi (i, (i+1) % NUMFILO) ;
    come (i) ;
    larga_hashi (i, i) ;
    larga_hashi (i, (i+1) % NUMFILO) ;
  }
  task_exit (0) ;
}

// programa principal
int main (int argc, char *argv[])
{
  int i, status ;
  ppos_init() ;

  // inicia os hashis
  for(i=0; i<NUMFILO; i++)
    sem_init (&hashi[i], 1) ;

  // inicia os filosofos
  for(i=0; i<NUMFILO; i++)
  {
    status = task_init (&filosofo[i], threadFilosofo, (void *) i) ;
    if (status < 0)
    {
      perror ("pthread_create") ;
      exit (1) ;
    }
  }

  // a main encerra aqui
  task_exit (0) ;
}

