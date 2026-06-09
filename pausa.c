#include "pausa.h"

/*
 * Implementação da função de pausa que adapta-se aos SOs diferentes
 * usando as diretivas de compilação condicional (#ifdef).
 */

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  
  // No Windows, usamos a função Sleep() da API padrão do Win32.
  void so_delay_ms(unsigned ms)
  {
      Sleep((DWORD)ms);
  }
#else
  #include <time.h>
  
  // Em sistemas baseados em Unix/Linux (POSIX), usamos a syscall nanosleep.
  // Note que aqui o nome mudou levemente, embora no uso pretendesse chamar-se so_delay_ms.
  void p_ms(unsigned ms)
  {
      struct timespec ts;
      ts.tv_sec  = (time_t)(ms / 1000);                      // Pega apenas a parte inteira dos segundos
      ts.tv_nsec = (long)(ms % 1000) * 1000000L;             // Converte o resto de milissegundos em nanossegundos
      nanosleep(&ts, 0); // Executa a suspensão da thread pelo tempo desejado
  }
#endif
