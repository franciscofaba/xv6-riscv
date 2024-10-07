#include "kernel/types.h"
#include "user/user.h"

void proceso_test(int id) {
  printf("Proceso %d ejecutado\n", id);
  sleep(10);  // Pausa por unos segundos para simular ejecución
}

int main() {
  int i;
  for (i = 0; i < 20; i++) {
    if (fork() == 0) {  // Crea un nuevo proceso
      proceso_test(i);  // Llama a la función de prueba
      exit(0);
    }
  }

  for (i = 0; i < 20; i++) {
    wait(0);  // Espera a que todos los hijos terminen
  }

  exit(0);
}