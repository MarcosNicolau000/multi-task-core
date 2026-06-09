#ifndef PAUSA_H
#define PAUSA_H

/*
 * Cabeçalhos para funções de utilidade de atraso (sleep/delay).
 * Usados caso os processos precisem de uma pausa temporal realista (em milissegundos).
 * Funciona de forma diferente dos semáforos, sendo uma pausa bloqueante por tempo de relógio.
 */

#ifdef __cplusplus
extern "C" {
#endif

// Assinatura da função para parar a execução por "ms" milissegundos.
void so_delay_ms(unsigned ms);

#ifdef __cplusplus
}
#endif

#endif // PAUSA_H
