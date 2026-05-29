#include "semaforo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void iniciaSemaforo(semaforo *s, int n) {
  if(!s) return;
    s->contadorSinc = n;
    s->filaBloqueados = NULL;
}

void P(semaforo *s) {
  PTR_DESC_PROC aux;
  PTR_DESC_PROC prox;
    if(!s) return;
    if(!atual){
        fprintf(stderr, "Nenhum processo ativo para executar P\n");
        exit(1);
    }
    if(s->contadorSinc > 0){
        s->contadorSinc--;
        return;
    } // Bloqueia o processo atual
    atual->estado = BLOQ_P;

    atual->filaSem = NULL;
    if(!s->filaBloqueados){
        s->filaBloqueados = atual;
    } else {
        aux = s->filaBloqueados;
        while(aux->filaSem){
            aux = aux->filaSem;
        }
        aux->filaSem = atual;
    }
    prox = NULL;
    if(primeiro){
        PTR_DESC_PROC scan = atual->proxDescritor;
        while(scan && scan != atual){
            if(scan->estado == ATIVO){
                prox = scan;
                break;
            }
            scan = scan->proxDescritor;
        }
        if (!prox && atual->estado == ATIVO) {
            prox = atual;
        }
    }
    if(!prox){
        fprintf(stderr, "Deadlock: todos processos estao bloqueados\n");
        exit(1);
    }
    yield();
}

void V(semaforo *s) {
  PTR_DESC_PROC p;
    if(!s) return;
    if(s->filaBloqueados == NULL){
        s->contadorSinc++;
        return;
    } else {
        p = s->filaBloqueados;
        s->filaBloqueados = p->filaSem;
        p->filaSem = NULL;
        p->estado = ATIVO;
    }
    
}
