#include "nucleo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PTR_DESC_PROC primeiro;
PTR_DESC_PROC atual;
static descritor mainDesc;
static PTR_DESC mainContext = &mainDesc;
static int mainPronto;

void iniciaFilaProntos(void) {
  primeiro = NULL;
  atual = NULL;
}

static void processoTrampolim(void *arg) {
  PTR_DESC_PROC proc = (PTR_DESC_PROC)arg;
  if (proc == NULL || proc->codigo == NULL) {
    printf("Processo invalido no trampolim.\n");
    exit(1);
  }

  proc->codigo();
  terminaProcesso();
}

void criaProcesso(void (*endProc)(void), const char *nomeProc) {
  PTR_DESC_PROC novoProcesso = (PTR_DESC_PROC)malloc(sizeof(descritorProc));

  if (!novoProcesso) {
    printf("Falha ao criar processo (erro no malloc)\n");
    exit(1);
  }

  strcpy(novoProcesso->nomeProcesso, nomeProc);
  novoProcesso->estado = ATIVO;
  novoProcesso->filaSem = NULL;
  novoProcesso->codigo = endProc;
  novoProcesso->contexto = cria_desc();

  newprocess(processoTrampolim, (void *)novoProcesso, novoProcesso->contexto);

  if (primeiro == NULL) {
    primeiro = novoProcesso;
    novoProcesso->proxDescritor = novoProcesso;
  } else {
    PTR_DESC_PROC temp = primeiro;
    while (temp->proxDescritor != primeiro) {
      temp = temp->proxDescritor;
    }
    temp->proxDescritor = novoProcesso;
    novoProcesso->proxDescritor = primeiro;
  }
}

static PTR_DESC_PROC ProximoAtivoDepois(PTR_DESC_PROC aPartir) {
  if (primeiro == NULL) {
    return NULL;
  }
  PTR_DESC_PROC inicio = (aPartir == NULL) ? primeiro : aPartir;
  PTR_DESC_PROC candidato =
      (aPartir == NULL) ? primeiro : aPartir->proxDescritor;

  do {
    if (candidato->estado == ATIVO) {
      return candidato;
    }
    candidato = candidato->proxDescritor;
  } while (candidato != inicio);

  if (inicio->estado == ATIVO) {
    return inicio;
  }

  return NULL;
}

void disparaSistema() {
  if (primeiro == NULL) {
    return;
  }
  system_init_main(mainContext);
  mainPronto = 1;
  if (primeiro->estado == ATIVO) {
    atual = primeiro;
  } else {
    atual = ProximoAtivoDepois(primeiro);
  }

  if (atual != NULL) {
    transfer(mainContext, atual->contexto);
  }
}

void yield() {
  if (atual == NULL) {
    return;
  }

  PTR_DESC_PROC proximoAtivo = ProximoAtivoDepois(atual);

  if (proximoAtivo != NULL && proximoAtivo != atual) {
    PTR_DESC_PROC antigo = atual;
    atual = proximoAtivo;
    transfer(antigo->contexto, atual->contexto);
  }
}

void terminaProcesso() {
  if (atual == NULL) {
    return;
  }

  atual->estado = TERMINADO;
  PTR_DESC_PROC proximoAtivo = ProximoAtivoDepois(atual);

  if (proximoAtivo == NULL) {
    PTR_DESC_PROC antigo = atual;
    atual = NULL;
    if (mainPronto == 1) {
      transfer(antigo->contexto, mainContext);
    } else {
      printf("ERRO, NAO PODE RETORNAR A MAIN");
      exit(1);
    }
  } else {
    PTR_DESC_PROC antigo = atual;
    atual = proximoAtivo;
    transfer(antigo->contexto, atual->contexto);
  }
}
