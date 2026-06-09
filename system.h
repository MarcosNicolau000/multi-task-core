#ifndef SYSTEM_H
#define SYSTEM_H

/* 
 * Implementação de co-rotinas (transferência de contexto) para compilador C no Windows.
 * Faz o uso da biblioteca de Fibers da API do Windows para simular os processos multitarefas.
 * Cada Fiber atua como uma thread cooperativa gerenciada manualmente pela nossa aplicação.
 */

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #error "Esta versao (fibers) foi preparada especificamente para Windows (_WIN32)."
#endif

// Define um tipo ponteiro de função usado para especificar o ponto de entrada das co-rotinas.
typedef void (*proc_fn)(void *arg);

/* 
 * Descritor de contexto (co-rotina) associado a uma Fiber.
 * Armazena a referência para a API do Windows, bem como a função a executar.
 */
typedef struct {
    LPVOID  fiber;   /* Handle gerado pelo Windows que identifica a Fiber */
    proc_fn proc;    /* Função de entrada que a Fiber executará */
    void   *arg;     /* Argumento genérico que será repassado para a função 'proc' */
} descritor;

typedef descritor* PTR_DESC;

/* 
 * Converte a thread atual (a main() original) em uma Fiber.
 * Obrigatório ser executado antes de tentar transferir controle, para que
 * possamos, eventualmente, retornar à main.
 */
void system_init_main(PTR_DESC d_main);

/* Aloca dinamicamente uma nova estrutura de descritor (contexto) em memória */
PTR_DESC cria_desc(void);

/* 
 * Cria efetivamente uma co-rotina (fiber) no Sistema Operacional.
 * Ela ficará pronta para iniciar na função 'proc' quando receber o controle por 'transfer'.
 */
void newprocess(proc_fn proc, void *arg, PTR_DESC d);

/* 
 * Função núcleo do escalonamento dependente de sistema.
 * Transfere o controle da CPU da fiber 'origem' para a fiber 'destino'.
 */
void transfer(PTR_DESC origem, PTR_DESC destino);

#endif
