// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Teste da tarefa main escalonável

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

#define WORKLOAD 40000

task_t Pang, Peng, Ping, Pong, Pung ;

// simula um processamento pesado
int hardwork (int n)
{
   int i, j, soma ;

   soma = 0 ;
   for (i=0; i<n; i++)
      for (j=0; j<n; j++)
         soma += j ;
   return (soma) ;
}

// corpo das threads
void Body (void * arg)
{
   printf ("%s: inicio em %4d ms\n", (char *) arg, systime()) ;
   hardwork (WORKLOAD) ;
   printf ("%s: fim    em %4d ms\n", (char *) arg, systime()) ;
   task_exit (0) ;
}

int main (int argc, char *argv[])
{
   ppos_init () ;

   printf ("main: inicio em %4d ms\n", systime()) ;

   task_init (&Pang, Body, "    Pang") ;
   task_init (&Peng, Body, "        Peng") ;
   task_init (&Ping, Body, "            Ping") ;
   task_init (&Pong, Body, "                Pong") ;
   task_init (&Pung, Body, "                    Pung") ;

   hardwork (0.75*WORKLOAD) ;

   printf ("main: fim    em %4d ms\n", systime()) ;
   task_exit (0);

   exit (0) ;
}
