#include "system.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum estadoProc { ATIVO, BLOQ_P, TERMINADO } estadoProc;

typedef struct descritorProc {
  char nomeProcesso[35];
  estadoProc estado;
  PTR_DESC contexto;
  struct descritorProc *filaSem;
  struct descritorProc *proxDescritor;
  void (*codigo)(void);

} descritorProc;

typedef descritorProc *PTR_DESC_PROC;

// INICIALIZA OS PONTEIROSDA FILA CIRCULAR
void iniciaFilaProntos(void);

// CRIA UM PCB E VINCULA À FUNÇÃO E INSERE NA FILA CIRCULAR
void criaProcesso(void (*fn)(void), const char *nome);

// CONVERTE O MAIN EM FIBER  E TRANSFERE O CONTROLE AO PRIMEIRO PROCESSO
void disparaSistema(void);

// PROCESSO ATUAL CEDE A CPU VOLUNTARAMENTE PARA O PRÓXIMO PROCESSO ATIVO
void yield(void);

void terminaProcesso(void);
