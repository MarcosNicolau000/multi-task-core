#include "semaforo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Inicializa a estrutura do semáforo.
// Atribui o valor inicial 'n' ao contador e zera a lista de processos bloqueados nele.
void iniciaSemaforo(semaforo *s, int n) {
  if(!s) return;
    s->contadorSinc = n;
    s->filaBloqueados = NULL; // A fila começa vazia.
}

// Operação P (Wait / Down)
// O processo solicita o uso do semáforo. Se o contador for > 0, o acesso é garantido imediatamente.
// Se não, o processo atual é bloqueado até que a operação V() correspondente seja chamada por outro processo.
void P(semaforo *s) {
  PTR_DESC_PROC aux;
  PTR_DESC_PROC prox;
  
    if(!s) return;
    if(!atual){ // Verificação de segurança: tem de existir um processo no contexto.
        fprintf(stderr, "Nenhum processo ativo para executar P\n");
        exit(1);
    }
    
    // Se o contador do semáforo for maior que zero, o recurso está livre.
    if(s->contadorSinc > 0){
        s->contadorSinc--; // Ocupa uma "vaga" do recurso.
        return;            // Segue a execução normalmente.
    } 
    
    // Se chegar aqui, contador é <= 0. O recurso não está disponível.
    // O processo atual sofrerá bloqueio de estado.
    atual->estado = BLOQ_P; // Altera o estado do processo para BLOQUEADO.

    // A seguir, insere o processo no FIM da fila de bloqueados específica deste semáforo.
    atual->filaSem = NULL;
    if(!s->filaBloqueados){
        // Se a fila do semáforo está vazia, ele é o primeiro.
        s->filaBloqueados = atual;
    } else {
        // Se a fila já contém processos, percorre até o fim.
        aux = s->filaBloqueados;
        while(aux->filaSem){
            aux = aux->filaSem;
        }
        // Insere no último lugar (comportamento de fila FIFO simples).
        aux->filaSem = atual;
    }
    
    // Agora que o processo bloqueou, o núcleo deve achar outro processo ATIVO para rodar,
    // caso contrário, o sistema para de rodar processos indevidamente.
    prox = NULL;
    if(primeiro){
        PTR_DESC_PROC scan = atual->proxDescritor;
        // Percorre a fila circular para achar um próximo processo ativo.
        while(scan && scan != atual){
            if(scan->estado == ATIVO){
                prox = scan; // Achou o processo para executar.
                break;
            }
            scan = scan->proxDescritor;
        }
        // Fallback: Se não encontrou ninguém e (improbabilisticamente) o próprio estivesse ATIVO.
        if (!prox && atual->estado == ATIVO) {
            prox = atual;
        }
    }
    
    // Se nenhum outro processo está ativo (todos bloqueados ou terminados).
    if(!prox){
        fprintf(stderr, "Deadlock: todos processos estao bloqueados\n");
        exit(1);
    }
    
    // Chama o yield() do núcleo. O yield vai suspender este processo bloqueado 
    // e o núcleo garantirá que 'prox' (ou o próximo da fila ativo) assuma a CPU.
    yield();
}

// Operação V (Signal / Up)
// O processo libera o recurso do semáforo que estava ocupando.
// Se existirem processos que se bloquearam na operação P(), ele retira e desperta o primeiro que aguardava.
// Se a fila de bloqueados estiver vazia, ele apenas aumenta os recursos disponíveis.
void V(semaforo *s) {
  PTR_DESC_PROC p;
  
    if(!s) return;
    
    // Se não há processos aguardando a liberação deste recurso:
    if(s->filaBloqueados == NULL){
        s->contadorSinc++; // Apenas incrementa o número de acessos livres.
        return;
    } else {
        // Se existem processos bloqueados aguardando este semáforo:
        p = s->filaBloqueados; // Pega o primeiro processo bloqueado da fila (FIFO).
        s->filaBloqueados = p->filaSem; // Avança a cabeça da fila.
        p->filaSem = NULL; // O processo retirado não está mais encadeado nesta fila.
        p->estado = ATIVO; // Altera o estado do processo retirado para ATIVO. 
                           // Ele voltará a competir pela CPU via escalonamento cooperativo (yield).
    }
}
