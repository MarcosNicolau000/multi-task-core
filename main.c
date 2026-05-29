#include <stdio.h>
#include <stdlib.h>
#include "nucleo.h"
#include "semaforo.h"

// TESTE 1: (Teste Básico de Escalonamento Cooperativo) - Imprime "1212..." alternadamente

void processo1() {
  while (1) {
    printf("1");
    yield();
  }
}

void processo2() {
  while (1) {
    printf("2");
    yield();
  }
}

// TESTE 2: Produtor/Consumidor com Semáforos (Implementação Completa)

#define TAM 5 
int buffer[TAM];
int in = 0;  
int out = 0; 

semaforo mutex; 
semaforo vazio; 
semaforo cheio; 

void Depositar(int mensagem) {
    buffer[in] = mensagem;
    printf("[PRODUTOR]   Depositou o item: %d (Posicao %d)\n", mensagem, in);
    in = (in + 1) % TAM;
}

int Retirar() {
    int mensagem = buffer[out];
    printf("[CONSUMIDOR] Retirou o item: %d (Posicao %d)\n", mensagem, out);
    out = (out + 1) % TAM;
    return mensagem;
}

void Produtor(void) {
    int item = 1; 
    for (int i = 0; i < 10; i++) { 
        P(&vazio);      
        P(&mutex);      
        Depositar(item);
        item++;
        V(&mutex);      
        V(&cheio);      
        yield();        
    }
    terminaProcesso(); 
}

void Consumidor(void) {
    for (int i = 0; i < 10; i++) { 
        P(&cheio);      
        P(&mutex);      
        Retirar();
        V(&mutex);      
        V(&vazio);      
        yield();        
    }
    terminaProcesso(); 
}

// MAIN - PONTO DE PARTIDA

int main(void) {
    printf("=== Iniciando Nucleo Multitarefas ===\n\n");

    iniciaFilaProntos();

    // TESTE 1 (Esalonamento Cooperativo Simples)
    /*
    criaProcesso(processo1, "proc1");
    criaProcesso(processo2, "proc2");
    printf("Sistema pronto. Iniciando teste simples 1212...\n");
    */

    // TESTE 2 (Produtor/Consumidor Final)
    
    iniciaSemaforo(&mutex, 1);    
    iniciaSemaforo(&vazio, TAM);  
    iniciaSemaforo(&cheio, 0);    

    criaProcesso(Produtor, "Produtor");
    criaProcesso(Consumidor, "Consumidor");
    printf("Sistema pronto. Iniciando Produtor/Consumidor...\n\n");
    

    // Dá a ignição no sistema
    disparaSistema();

    printf("\n=== Sistema finalizado com sucesso. De volta a Main. ===\n");
    return 0;
}