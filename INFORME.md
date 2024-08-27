# Pasos para instalar xv6 con QEMU
1. Cloné el repositorio: $ git clone https://github.com/otrab/xv6-riscv
2. Dado que estoy en una distribución de Kali Linux basada en Debian, utilicé el siguiente comando para instalar las dependencias necesarias: $ sudo apt-get update && sudo apt-get install git nasm build-essential qemu gdb
3. Me dirigí a la carpeta de xv6: $ cd xv6-riscv
4. Ejecuté el siguiente comando para iniciar xv6 con QEMU: $ make qemu

# Problemas:
-Mi principal problema fue darme cuenta de cuál era la instalación que realmente debía realizar. Una vez que comprendí que Kali está basado en Debian, fue mucho más fácil lograrlo.


