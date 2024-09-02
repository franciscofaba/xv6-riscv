dificultades:
Una de las cosas que mas me dificulto fue entender que habian dos camino para implementar la funcion getppid uno en c y el otro en assembly dentro del archivo usys.S, esto mehizo perder mucho tiempo.

pasos:
1. Definir la syscall en syscall.h:
	
	#define SYS_getppid 22 

En este paso, se define la nueva syscall SYS_getppid y le asigna un número único (en este caso, el 22). Este número es crucial porque el kernel lo utiliza para identificar la syscall cuando se realiza una llamada. La definición en syscall.h permite al kernel saber que existe una nueva syscall con ese número específico.

2. Implementar la syscall sys_getppid en sysproc.c:

	int sys_getppid(void)
	{
    		return myproc()->parent->pid;
	}	


Aquí, se implementa la lógica de la syscall getppid. En xv6, cada proceso tiene un puntero al proceso padre (parent). La función myproc() obtiene el proceso actual, y myproc()->parent->pid devuelve el ID del proceso padre (PPID). Esta implementación asegura que la syscall getppid devuelva el ID del proceso padre cuando sea llamada.

3. Registrar la syscall en syscall.c:

	extern uint64 sys_getppid(void);

	[SYS_getppid] sys_getppid,

En este paso, se registra la syscall en la tabla de syscalls en syscall.c. Primero, se declara externamente la función sys_getppid, que fue implementada en el paso anterior. Luego, se asocia el número de syscall (SYS_getppid) con la función sys_getppid. Esto permite al kernel mapear el número de syscall con la función específica que debe ejecutar cuando se llama.

4. Crear la declaración en un archivo de encabezado (user.h):

	int getppid(void);
	int syscall(int num, ...);

Aquí, el se crea la declaración de la función getppid en user.h para que esté disponible en el espacio de usuario. También se declara la función syscall, que se utiliza para realizar llamadas a syscalls específicas desde el espacio de usuario. Este paso es importante porque permite que los programas de usuario conozcan y puedan invocar la nueva syscall.

5. Uso en el código getppid:

	#include "types.h"
	#include "stat.h"
	#include "user.h"

	int getppid(void)
	{
    		return syscall(SYS_getppid);
	}


En este paso, se define la función getppid en el espacio de usuario. Esta función simplemente invoca la syscall pasando el número SYS_getppid. De esta manera, cuando un programa de usuario llama a getppid(), se realiza la llamada a la syscall correspondiente en el kernel.

6. Implementar getppid en usys.S:

	.global getppid
	getppid:
    	li a7, SYS_getppid  
    	ecall               
    	ret                 


Finalmente, se implementa la función getppid en ensamblador dentro de usys.S. Esta función carga el número de syscall SYS_getppid en el registro a7, que es donde el kernel espera encontrar el número de syscall, y luego ejecuta la instrucción ecall, que realiza la transición del espacio de usuario al espacio del kernel. La instrucción ret devuelve el control al código de usuario una vez que el kernel ha procesado la syscall.



