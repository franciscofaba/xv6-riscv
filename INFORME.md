PRIMERA ITERACION/COMMIT PARA HACER LA TAREA:

1.- Modificación de la estructura del proceso: 

    para implementar un sistema de prioridades, cada proceso necesita tener una prioridad que determine cuándo debe ser ejecutado en relación con otros procesos. En xv6, las propiedades de cada proceso se almacenan/implementan en la estructura "struct proc", definida en "proc.h".

    - priority: Al agregar esta variable, cada proceso tendrá un número que indica su prioridad. En este caso, estamos usando una convención donde un número menor significa mayor prioridad.
    - boost: Esta variable es necesaria porque las prioridades van a cambiar a lo largo del tiempo. El boost permite que las prioridades se ajusten dinámicamente. Si el boost es positivo, la prioridad del proceso sube; si es negativo, la prioridad baja. Esto ayuda a balancear la ejecución entre procesos.

    Finalmente asi quedo el struct proc del archivo proc.h con la prioridad y el boost:
    
    struct proc {
        struct spinlock lock;

        // p->lock must be held when using these:
        enum procstate state;        // Process state
        void *chan;                  // If non-zero, sleeping on chan
        int killed;                  // If non-zero, have been killed
        int xstate;                  // Exit status to be returned to parent's wait
        int pid;                     // Process ID

        // wait_lock must be held when using this:
        struct proc *parent;         // Parent process

        // New fields for priority scheduling
        int priority;   // Process priority (0 = highest priority)
        int boost;      // Boost for dynamic adjustment (1 or -1)

        // these are private to the process, so p->lock need not be held.
        uint64 kstack;               // Virtual address of kernel stack
        uint64 sz;                   // Size of process memory (bytes)
        pagetable_t pagetable;       // User page table
        struct trapframe *trapframe; // data page for trampoline.S
        struct context context;      // swtch() here to run process
        struct file *ofile[NOFILE];  // Open files
        struct inode *cwd;           // Current directory
        char name[16];               // Process name (debugging)
        };

2.- Inicialización en allocproc():

    La función allocproc() en el archivo proc.c se encarga de asignar memoria y configurar los valores iniciales cuando se crea un nuevo proceso. El objetivo es que todos los procesos inicien con los mismos valores de prioridad (0) y boost (1).


    static struct proc*
    allocproc(void)
    {
        struct proc *p;

        for(p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
        if(p->state == UNUSED) {
            goto found;
            } else {
            release(&p->lock);
            }
        }
        return 0;

        found:
        p->pid = allocpid();
        p->state = USED;

        // Inicialización de prioridad y boost
        p->priority = 0;  // Prioridad inicial
        p->boost = 1;     // Boost inicial

        // Allocate a trapframe page.
        if((p->trapframe = (struct trapframe *)kalloc()) == 0){
        freeproc(p);
        release(&p->lock);
        return 0;
        }

        // An empty user page table.
        p->pagetable = proc_pagetable(p);
        if(p->pagetable == 0){
        freeproc(p);
        release(&p->lock);
        return 0;
        }

        // Set up new context to start executing at forkret,
        // which returns to user space.
        memset(&p->context, 0, sizeof(p->context));
        p->context.ra = (uint64)forkret;
        p->context.sp = p->kstack + PGSIZE;

        return p;  
}


3.- Modificación del planificador (scheduler()) para manejar prioridades:

El scheduler es responsable de decidir qué proceso debe ejecutarse. Sin un sistema de prioridades, xv6 selecciona los procesos basándose en un esquema round-robin que es justo pero no tiene en cuenta ninguna noción de importancia entre procesos. Para darle mayor prioridad a los procesos con mayor importancia se debe modifique la funcion scheduler() del archivo proc.c:

  1.- Dentro de la funcion scheduler() hay que aumentar la prioridad de los procesos no zombies.
  2.- Luego, modificar el planificador para que siempre seleccione el proceso con la mayor prioridad (es decir, el que tiene el valor de prioridad más bajo). Para hacer esto, el codigo recorre la lista de procesos y compara las prioridades, seleccionando el proceso con la menor prioridad.

    void
    scheduler(void)
    {
      struct proc *p;
      struct proc *selected_proc = 0;
      struct cpu *c = mycpu();
      int highest_priority;
    
      c->proc = 0;
      for(;;){
        // The most recent process to run may have had interrupts
        // turned off; enable them to avoid a deadlock if all
        // processes are waiting.
        intr_on();
    
        highest_priority = 10;  // Inicializamos con una prioridad mayor
    
        // Primero aumentamos la prioridad de todos los procesos RUNNABLE
        for(p = proc; p < &proc[NPROC]; p++) {
          acquire(&p->lock);
          if(p->state == RUNNABLE) {  // Solo modificar RUNNABLE
            p->priority += p->boost;  // Ajustar prioridad por boost
    
            // Cambiar boost según los límites de prioridad
            if(p->priority >= 9)
              p->boost = -1;  // Si la prioridad llega a 9, comienza a disminuir
            else if(p->priority <= 0)
              p->boost = 1;   // Si la prioridad es 0, comienza a aumentar
          }
          release(&p->lock);
        }
    
        // Ahora seleccionamos el proceso con la mayor prioridad (el valor más bajo)
        for(p = proc; p < &proc[NPROC]; p++) {
          acquire(&p->lock);
          if(p->state == RUNNABLE && p->priority < highest_priority) {
            highest_priority = p->priority;
            selected_proc = p;
          }
          release(&p->lock);
        }
    
        // Si encontramos un proceso seleccionado, lo ejecutamos
        if (selected_proc) {
          acquire(&selected_proc->lock);
          selected_proc->state = RUNNING;
          c->proc = selected_proc;
          swtch(&c->context, &selected_proc->context);
    
          // El proceso ha terminado de ejecutarse por ahora.
          c->proc = 0;
          release(&selected_proc->lock);
        } else {
          // Si no hay procesos RUNNABLE, entra en espera
          intr_on();
          asm volatile("wfi");
        }
      }
    }

4.- Crear el programa de prueba:

  flujo del programa:

    1.- Se ejecuta el proceso principal (padre).
    2.- En el primer bucle for, el proceso principal crea 20 procesos hijos utilizando fork().
    3.- Cada proceso hijo ejecuta proceso_test(i) para imprimir su número de proceso y luego se duerme por 10 ticks antes de finalizar.
    4.- Mientras tanto, el proceso padre ejecuta el segundo bucle for, donde espera que cada uno de los procesos hijos termine su ejecución usando wait(0).
    5.- Cuando todos los procesos hijos han terminado, el proceso padre también termina.
  
  codigo:

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

    
5.- Al compilar obtuve el error:
    
        mkfs: mkfs/mkfs.c:150: main: Assertion `strlen(shortname) <= DIRSIZ' failed.

    para solucionarlo cambie el nombre a pprocess.c con 8 caracteres.

6.-  El output del script fue:

    init: starting sh
    $ pprocess
    PrProceocesPrPsrocProceso 4o eso 3 ejecutadoejecuta
    dProceso Pro
    Proceso 7 ejecutado
    Proceso 8 ejecutado
    Proceso 9 ejecutado
    PProceso 11 ejecutado
    Proceso 10 ejec5utoceso 6 ejecutado
    Proceso 13 ejecutadroadoc
    eo
    Proceso 16 ejecutadoP
    ProPceso 15 ejecutaPrdo
    Proceoso 0co  ejecu19res ejecutado
    o o t18ado eje
    cutceso 2 e1 eje cado
    jecuutado
    esoroceso 12 toeceso  ado
    jecutado
    14 17ejec ejeutcutadoado
    
    jecutado
    $ 
    
    