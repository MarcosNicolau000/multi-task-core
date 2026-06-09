#ifndef SEMAFORO_H
#define SEMAFORO_H

#include "nucleo.h"

// Estrutura que define um semáforo de Dijkstra.
// É usado para sincronização entre processos (ex: Produtor/Consumidor) e exclusão mútua.
typedef struct semaforo {
  int contadorSinc;               // Valor inteiro do semáforo. >0 significa que o recurso está livre.
                                  // <=0 significa recurso ocupado e a quantidade em módulo indica quantos estão bloqueados.
  PTR_DESC_PROC filaBloqueados;   // Ponteiro que serve de cabeça para a fila de processos que estão bloqueados esperando neste semáforo.
} semaforo;

// Função que inicializa o semáforo com um valor especificado (recursos disponíveis).
void iniciaSemaforo(semaforo *s, int n);

// P (Wait ou Down) - Abreviação de "Proberen" (testar).
// Operação usada para solicitar o acesso a um recurso controlado pelo semáforo.
// Se o contador for maior que 0, decrementa e permite o acesso.
// Caso contrário (contador <= 0), bloqueia o processo atual e o insere na fila de bloqueados do semáforo.
void P(semaforo *s); 

// V (Signal ou Up) - Abreviação de "Verhogen" (incrementar).
// Operação usada para liberar um recurso controlado pelo semáforo.
// Se houver processos bloqueados aguardando na fila, acorda o mais antigo (muda para ATIVO).
// Caso contrário, apenas incrementa o contador do semáforo.
void V(semaforo *s); 

#endif 
