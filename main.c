#include "nucleo.h"
#include <stdio.h>

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

int main(void) {
  iniciaFilaProntos();
  criaProcesso(processo1, "proc1");
  criaProcesso(processo2, "proc2");
  printf("Sistema pronto. Iniciando escalonador cooperativo...\n");
  disparaSistema();
  printf("\nSistema finalizado com sucesso.\n");
  return 0;
}