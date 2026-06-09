#include <stdio.h>
#include <stdlib.h>
#include "nucleo.h"
#include "semaforo.h"

// TESTE 1: (Teste Básico de Escalonamento Cooperativo) - Imprime "1212..." alternadamente

// Função que simula o primeiro processo.
// Ele executa um loop infinito imprimindo "1" e, em seguida, cede a CPU.
void processo1() {
  while (1) {
    printf("1");
    // yield() pausa este processo voluntariamente e passa o controle para o próximo da fila.
    yield();
  }
}

// Função que simula o segundo processo.
// Similar ao processo1, mas imprime "2".
void processo2() {
  while (1) {
    printf("2");
    // Cede a CPU para que o processo1 (ou outro) possa executar.
    yield();
  }
}

// TESTE 2: Produtor/Consumidor com Semáforos (Implementação Completa)

#define TAM 5 
int buffer[TAM]; // Buffer circular que armazena os itens produzidos.
int in = 0;      // Índice onde o próximo item será depositado (Produzido).
int out = 0;     // Índice de onde o próximo item será retirado (Consumido).

// Semáforos para controle de concorrência e sincronização:
semaforo mutex;  // Garante exclusão mútua ao acessar o buffer (protege a região crítica).
semaforo vazio;  // Controla o número de espaços vazios no buffer. Bloqueia o produtor se o buffer estiver cheio.
semaforo cheio;  // Controla o número de espaços ocupados (itens) no buffer. Bloqueia o consumidor se o buffer estiver vazio.

// Função auxiliar usada pelo Produtor para inserir um item no buffer.
void Depositar(int mensagem) {
    buffer[in] = mensagem;
    printf("[PRODUTOR]   Depositou o item: %d (Posicao %d)\n", mensagem, in);
    in = (in + 1) % TAM; // Avança o índice de forma circular.
}

// Função auxiliar usada pelo Consumidor para ler e remover um item do buffer.
int Retirar() {
    int mensagem = buffer[out];
    printf("[CONSUMIDOR] Retirou o item: %d (Posicao %d)\n", mensagem, out);
    out = (out + 1) % TAM; // Avança o índice de forma circular.
    return mensagem;
}

// Lógica do processo Produtor.
void Produtor(void) {
    int item = 1; // Inicia a contagem de itens a serem produzidos.
    for (int i = 0; i < 10; i++) { // Produzirá 10 itens no total.
        P(&vazio);      // Aguarda até ter pelo menos um espaço vazio no buffer. (Decrementa 'vazio')
        P(&mutex);      // Entra na região crítica para acessar o buffer de forma exclusiva.
        
        Depositar(item); // Deposita o item gerado no buffer.
        item++;          // Prepara o próximo item a ser produzido.
        
        V(&mutex);      // Sai da região crítica, liberando o buffer para outros processos.
        V(&cheio);      // Sinaliza que um novo item foi adicionado. (Incrementa 'cheio')
        
        yield();        // Cede a CPU para permitir que o Consumidor (ou outro processo) execute.
    }
    terminaProcesso(); // Encerra o processo produtor ao terminar sua tarefa.
}

// Lógica do processo Consumidor.
void Consumidor(void) {
    for (int i = 0; i < 10; i++) { // Consumirá 10 itens no total.
        P(&cheio);      // Aguarda até ter pelo menos um item disponível no buffer. (Decrementa 'cheio')
        P(&mutex);      // Entra na região crítica para acessar o buffer de forma exclusiva.
        
        Retirar();      // Retira o item do buffer.
        
        V(&mutex);      // Sai da região crítica, liberando o buffer para outros processos.
        V(&vazio);      // Sinaliza que um novo espaço vazio foi liberado. (Incrementa 'vazio')
        
        yield();        // Cede a CPU para permitir que o Produtor (ou outro processo) execute.
    }
    terminaProcesso(); // Encerra o processo consumidor ao terminar sua tarefa.
}

// MAIN - PONTO DE PARTIDA

// Função principal que inicializa o núcleo e agenda os processos iniciais.
int main(void) {
    printf("=== Iniciando Nucleo Multitarefas ===\n\n");

    // Prepara as estruturas internas do núcleo (fila de prontos).
    iniciaFilaProntos();

    // TESTE 1 (Escalonamento Cooperativo Simples)
    // Código comentado que demonstra o uso básico sem sincronização (apenas prints alternados).
    /*
    criaProcesso(processo1, "proc1");
    criaProcesso(processo2, "proc2");
    printf("Sistema pronto. Iniciando teste simples 1212...\n");
    */

    // TESTE 2 (Produtor/Consumidor Final)
    
    // Inicialização dos semáforos:
    iniciaSemaforo(&mutex, 1);    // Mutex inicia em 1: recurso livre para o primeiro que chegar.
    iniciaSemaforo(&vazio, TAM);  // Vazio inicia com TAM: todos os espaços do buffer estão vazios inicialmente.
    iniciaSemaforo(&cheio, 0);    // Cheio inicia em 0: nenhum item disponível inicialmente.

    // Cria os processos na fila de prontos do núcleo.
    criaProcesso(Produtor, "Produtor");
    criaProcesso(Consumidor, "Consumidor");
    printf("Sistema pronto. Iniciando Produtor/Consumidor...\n\n");
    
    // Dá a ignição no sistema: transfere o controle para o primeiro processo criado.
    // O retorno dessa função só ocorre quando não houver mais processos ATIVOS.
    disparaSistema();

    printf("\n=== Sistema finalizado com sucesso. De volta a Main. ===\n");
    return 0;
}
