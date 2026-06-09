#include "system.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * Ponto de entrada real que a API do Windows chama ao iniciar a Fiber.
 * Extrai o contexto e repassa para a função final do usuário.
 */
static void __stdcall fiber_entry(void *param)
{
    PTR_DESC d = (PTR_DESC)param;
    if (!d || !d->proc) {
        fprintf(stderr, "fiber_entry: descritor/proc invalido.\n");
        ExitProcess(1);
    }

    // Chama a função designada para o processo (nosso "trampolim")
    d->proc(d->arg);

    /* 
     * Se a execução da função retornar, significa que o processo do usuário
     * acabou sua função sem invocar a função "terminaProcesso" que lidaria com a troca para a main.
     * Na nossa arquitetura, isto é considerado um erro, pois o núcleo multitarefa
     * deveria gerenciar a morte do processo.
     */
    fprintf(stderr, "Erro: co-rotina retornou. Use termina_processo() ao final do processo.\n");
    ExitProcess(1);
}

/*
 * Inicializa a thread base (onde a main executa) transformando-a na primeira Fiber do programa.
 */
void system_init_main(PTR_DESC d_main)
{
    if (!d_main) {
        fprintf(stderr, "system_init_main: d_main nulo.\n");
        exit(1);
    }

    if (d_main->fiber != NULL) {
        /* Já inicializado, evita inicialização duplicada */
        return;
    }

    /* 
     * Converte a thread atual do Sistema Operacional (main) em Fiber. 
     * Isto é crucial para que possamos usar "SwitchToFiber" a partir da main.
     */
    d_main->fiber = ConvertThreadToFiber(NULL);
    if (d_main->fiber == NULL) {
        fprintf(stderr, "ConvertThreadToFiber falhou. GetLastError=%lu\n", (unsigned long)GetLastError());
        exit(1);
    }

    // A main não tem uma função de usuário, seu corpo é a própria função atual.
    d_main->proc = NULL;
    d_main->arg  = NULL;
}

/*
 * Prepara a memória e aloca um descritor vazio (inicializado com zeros).
 */
PTR_DESC cria_desc(void)
{
    PTR_DESC d = (PTR_DESC)calloc(1, sizeof(descritor));
    if (!d) {
        perror("cria_desc"); // Mensagem de erro padrão se memória esgotar
        exit(1);
    }
    return d;
}

/*
 * Invoca a API do Windows para instanciar a fiber para um novo processo.
 */
void newprocess(proc_fn proc, void *arg, PTR_DESC d)
{
    if (!d || !proc) {
        fprintf(stderr, "newprocess: argumentos invalidos.\n");
        exit(1);
    }

    d->proc = proc; // Função que a co-rotina irá executar
    d->arg  = arg;  // Parâmetros a serem passados a essa função

    /* 
     * CreateFiber aloca o contexto do SO para esta co-rotina.
     * stackSize=0 faz com que adote o tamanho default da pilha do executável.
     * fiber_entry é o ponto de interceptação local da nossa lib, e 'd' será seu parâmetro.
     */
    d->fiber = CreateFiber(0, fiber_entry, d);
    if (d->fiber == NULL) {
        fprintf(stderr, "CreateFiber falhou. GetLastError=%lu\n", (unsigned long)GetLastError());
        exit(1);
    }
}

/*
 * Executa a troca do contexto ativo. 
 * O SO suspende imediatamente a Fiber atual e passa a executar as instruções 
 * no contexto da Fiber contida em 'destino'.
 */
void transfer(PTR_DESC origem, PTR_DESC destino)
{
    /* 
     * O parâmetro origem é mantido apenas por compatibilidade com interfaces POSIX (ucontext).
     * Nas Fibers do Windows, SwitchToFiber(destino) automaticamente salva o estado da fiber atual,
     * então 'origem' pode ser silenciosamente ignorado aqui.
     */
    (void)origem; 

    if (!destino || !destino->fiber) {
        fprintf(stderr, "transfer: destino invalido.\n");
        exit(1);
    }

    // API do Windows que efetivamente realiza a troca de contexto entre co-rotinas
    SwitchToFiber(destino->fiber);
}
