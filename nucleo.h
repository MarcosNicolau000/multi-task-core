#ifndef NUCLEO_H
#define NUCLEO_H

#include "system.h"
#include <stdio.h>
#include <stdlib.h>

// Define os possíveis estados em que um processo pode estar durante sua vida útil.
typedef enum estadoProc { 
  ATIVO,      // Processo está executando ou pronto para executar.
  BLOQ_P,     // Processo está bloqueado, aguardando liberação de um semáforo (operação P).
  TERMINADO   // Processo finalizou sua execução e não deve mais ser escalonado.
} estadoProc;

// Estrutura do Bloco de Controle de Processo (PCB - Process Control Block).
// Armazena as informações necessárias para gerenciar cada processo.
typedef struct descritorProc {
  char nomeProcesso[35];          // Nome identificador do processo para fins de log/debug.
  estadoProc estado;              // Estado atual do processo (ATIVO, BLOQ_P, TERMINADO).
  PTR_DESC contexto;              // Ponteiro para o contexto de execução dependente da plataforma (Windows Fibers).
  struct descritorProc *filaSem;  // Ponteiro usado para encadear o processo na fila de um semáforo (se bloqueado).
  struct descritorProc *proxDescritor; // Ponteiro para o próximo processo na fila circular de escalonamento.
  void (*codigo)(void);           // Ponteiro para a função C que contém o código do processo a ser executado.
} descritorProc;

// Define um tipo de ponteiro para facilitar o uso da estrutura descritorProc.
typedef descritorProc *PTR_DESC_PROC;

// Exportando as variáveis globais para o semaforo.c conseguir manipular.
// "primeiro" aponta para o primeiro elemento inserido na fila circular de processos.
extern PTR_DESC_PROC primeiro;
// "atual" aponta para o processo que está atualmente de posse da CPU.
extern PTR_DESC_PROC atual;

// INICIALIZA OS PONTEIROS DA FILA CIRCULAR
// Deve ser chamada antes de criar qualquer processo para preparar o ambiente.
void iniciaFilaProntos(void);

// CRIA UM PCB, VINCULA À FUNÇÃO E INSERE NA FILA CIRCULAR
// Recebe um ponteiro para a função do processo e o nome do processo.
void criaProcesso(void (*fn)(void), const char *nome);

// CONVERTE A THREAD DA MAIN EM FIBER E TRANSFERE O CONTROLE AO PRIMEIRO PROCESSO
// Dá a ignição no núcleo multitarefas; só retorna à main quando todos terminarem.
void disparaSistema(void);

// PROCESSO ATUAL CEDE A CPU VOLUNTARIAMENTE PARA O PRÓXIMO PROCESSO ATIVO
// Usada no escalonamento cooperativo para que os processos dividam o tempo.
void yield(void);

// FUNÇÃO PARA ENCERRAR UM PROCESSO
// Marca o processo atual como TERMINADO e passa o controle para o próximo ativo.
void terminaProcesso(void);

#endif // NUCLEO_H
