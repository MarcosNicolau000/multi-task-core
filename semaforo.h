#ifndef SEMAFORO_H
#define SEMAFORO_H

#include "nucleo.h"
typedef struct semaforo {
  int contadorSinc;
  PTR_DESC_PROC filaBloqueados;
} semaforo;

void iniciaSemaforo(semaforo *s, int n);

void P(semaforo *s); //Down/Wait solicita recursos, se não tiver, bloqueia o processo

void V(semaforo *s); // Up/Signal libera recursos, se tiver processo bloqueado, libera o processo mais antigo da fila de bloqueados

#endif 