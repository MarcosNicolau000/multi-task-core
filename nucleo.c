#include "nucleo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variáveis globais do núcleo multitarefas.
PTR_DESC_PROC primeiro; // Ponteiro para o primeiro processo criado na fila circular.
PTR_DESC_PROC atual;    // Ponteiro para o processo atualmente em execução.

// Variáveis de controle para o contexto da função main().
static descritor mainDesc;                // Estrutura que guardará o contexto da função main.
static PTR_DESC mainContext = &mainDesc;  // Ponteiro fixo para o contexto da main.
static int mainPronto;                    // Flag que indica se a main já foi convertida para Fiber.

// Inicializa a fila de processos (Fila Circular).
// Chamada na main antes da criação de qualquer processo.
void iniciaFilaProntos(void) {
  primeiro = NULL; // A fila começa vazia.
  atual = NULL;    // Nenhum processo executando ainda.
}

// Função intermediária "trampolim" que envolve a execução do processo.
// Ela chama o código do processo e garante que terminaProcesso() seja invocado ao final,
// evitando que a co-rotina (Fiber) retorne de forma ilegal.
static void processoTrampolim(void *arg) {
  PTR_DESC_PROC proc = (PTR_DESC_PROC)arg;
  
  if (proc == NULL || proc->codigo == NULL) {
    printf("Processo invalido no trampolim.\n");
    exit(1);
  }

  // Executa o código real definido para o processo (ex: Produtor ou Consumidor).
  proc->codigo();
  
  // Ao terminar o loop principal do processo, encerra sua participação no núcleo.
  terminaProcesso();
}

// Função que cria um novo processo (Bloco de Controle de Processo - PCB)
// e o enfileira na estrutura circular de processos prontos.
void criaProcesso(void (*endProc)(void), const char *nomeProc) {
  // Aloca memória dinamicamente para o descritor do processo.
  PTR_DESC_PROC novoProcesso = (PTR_DESC_PROC)malloc(sizeof(descritorProc));

  if (!novoProcesso) {
    printf("Falha ao criar processo (erro no malloc)\n");
    exit(1);
  }

  // Preenche os dados do PCB.
  strcpy(novoProcesso->nomeProcesso, nomeProc);
  novoProcesso->estado = ATIVO;      // Processo recém-criado já nasce apto a rodar.
  novoProcesso->filaSem = NULL;      // Não está bloqueado em nenhum semáforo ainda.
  novoProcesso->codigo = endProc;    // Endereço de memória da função a ser executada.
  novoProcesso->contexto = cria_desc(); // Cria o contexto dependente de plataforma.

  // Vincula a função trampolim ao contexto criado, passando o PCB do processo como argumento.
  newprocess(processoTrampolim, (void *)novoProcesso, novoProcesso->contexto);

  // Inserção na fila circular encadeada.
  if (primeiro == NULL) {
    // Se for o primeiro processo criado, aponta para si mesmo.
    primeiro = novoProcesso;
    novoProcesso->proxDescritor = novoProcesso;
  } else {
    // Se não for o primeiro, percorre a fila até achar o último nó...
    PTR_DESC_PROC temp = primeiro;
    while (temp->proxDescritor != primeiro) {
      temp = temp->proxDescritor;
    }
    // ... e o insere no final, fechando o círculo apontando para "primeiro".
    temp->proxDescritor = novoProcesso;
    novoProcesso->proxDescritor = primeiro;
  }
}

// Função auxiliar de escalonamento que busca o próximo processo ATIVO na fila circular.
// Inicia a busca logo após o processo "aPartir".
static PTR_DESC_PROC ProximoAtivoDepois(PTR_DESC_PROC aPartir) {
  if (primeiro == NULL) {
    return NULL; // Fila vazia
  }
  
  PTR_DESC_PROC inicio = (aPartir == NULL) ? primeiro : aPartir;
  PTR_DESC_PROC candidato = (aPartir == NULL) ? primeiro : aPartir->proxDescritor;

  // Percorre toda a fila circular procurando por um processo que não esteja BLOQUEADO ou TERMINADO.
  do {
    if (candidato->estado == ATIVO) {
      return candidato; // Achou um processo ativo.
    }
    candidato = candidato->proxDescritor;
  } while (candidato != inicio);

  // Se der a volta completa e o próprio "inicio" for o único ativo, retorna ele.
  if (inicio->estado == ATIVO) {
    return inicio;
  }

  // Se não encontrar ninguém ativo, retorna NULL.
  return NULL;
}

// Dá a ignição do núcleo multitarefas.
// Converte a main para um Fiber e faz a primeira transferência de contexto.
void disparaSistema() {
  if (primeiro == NULL) {
    return; // Se não houver processos, apenas retorna para a main.
  }
  
  // Inicializa a thread da main como um Fiber, para que depois ela possa ser retomada.
  system_init_main(mainContext);
  mainPronto = 1;
  
  // Define qual será o primeiro processo a rodar.
  if (primeiro->estado == ATIVO) {
    atual = primeiro;
  } else {
    atual = ProximoAtivoDepois(primeiro);
  }

  // Se achou alguém apto, realiza a troca de contexto da main para o processo escolhido.
  if (atual != NULL) {
    transfer(mainContext, atual->contexto);
  }
}

// Função de Escalonamento Cooperativo: cede a posse da CPU voluntariamente.
void yield() {
  if (atual == NULL) {
    return;
  }

  // Procura o próximo processo ativo na fila circular após o processo atual.
  PTR_DESC_PROC proximoAtivo = ProximoAtivoDepois(atual);

  // Se existe um processo ativo diferente do atual, faz a troca.
  // Se só houver o processo atual ativo, ele continua rodando.
  if (proximoAtivo != NULL && proximoAtivo != atual) {
    PTR_DESC_PROC antigo = atual;
    atual = proximoAtivo;
    
    // Troca de contexto: salva o estado do atual e carrega o do próximo.
    transfer(antigo->contexto, atual->contexto);
  }
}

// Função para indicar que o processo atual concluiu sua execução.
void terminaProcesso() {
  if (atual == NULL) {
    return;
  }

  // Muda o estado do processo para TERMINADO, tirando-o do escalonamento.
  atual->estado = TERMINADO;
  
  // Procura o próximo processo ativo para dar a vez a ele.
  PTR_DESC_PROC proximoAtivo = ProximoAtivoDepois(atual);

  if (proximoAtivo == NULL) {
    // Se não houver mais nenhum processo ativo (ou todos terminaram ou estão em deadlock permanente),
    // retorna a execução para a função main.
    PTR_DESC_PROC antigo = atual;
    atual = NULL;
    if (mainPronto == 1) {
      transfer(antigo->contexto, mainContext);
    } else {
      printf("ERRO, NAO PODE RETORNAR A MAIN");
      exit(1);
    }
  } else {
    // Caso haja outro processo ativo, passa o controle para ele.
    PTR_DESC_PROC antigo = atual;
    atual = proximoAtivo;
    transfer(antigo->contexto, atual->contexto);
  }
}
