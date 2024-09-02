#include "user.h"

int main(void) {
    int ppid = getppid();
    printf("Parent process ID: %d\n", ppid);
    exit(0);
}
