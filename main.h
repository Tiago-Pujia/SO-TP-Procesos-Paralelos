#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

/*
===================================================
          INCLUSI�N DE BIBLIOTECAS EST�NDAR
===================================================
*/

#include <stdio.h>      // Para funciones de entrada/salida como printf, perror, etc.
#include <stdlib.h>     // Para funciones como malloc, free, exit, rand, etc.
#include <string.h>     // Para funciones de manipulaci�n de cadenas
#include <fcntl.h>      // Para constantes como O_CREAT, O_RDWR usadas en shm_open
#include <sys/mman.h>   // Para funciones de mapeo de memoria compartida (mmap, munmap)
#include <unistd.h>     // Para funciones como close, usleep, fork, etc.
#include <semaphore.h>  // Para manejo de sem�foros POSIX
#include <time.h>       // Para manejo de tiempo y fechas (time, time_t, srand, etc.)
#include <errno.h>      // Para la gesti�n de errores del sistema
#include <sys/stat.h>   // Para permisos de archivos y memoria compartida
#include <signal.h>     // Para manejo de se�ales como SIGTERM
#include <sys/wait.h>   // Para espera de procesos hijos (waitpid)

/*
===================================================
                   DEFINICIONES
===================================================
*/

#define NUM_BLOQUES 10          // N�mero total de bloques en el vector compartido
#define TAM_VECTOR 100          // Tama�o total del vector de enteros compartido
#define NUM_HIJOS 6             // N�mero total de procesos hijos que se crear�n
#define BLOQUES_POR_HIJO 3      // Cantidad de bloques que gestiona cada hijo

/*
===================================================
     ESTRUCTURA DE CONFIGURACI�N DEL HIJO
===================================================
*/

/**
 * Estructura que define la configuraci�n individual de un proceso hijo.
 */
typedef struct
{
    int pid;                            // PID del proceso hijo (usado para seguimiento)
    char operacion;                    // C�digo de operaci�n a ejecutar (e.g. 'M', 'P', etc.)
    int frecuencia;                    // Frecuencia en segundos entre ciclos de trabajo
    int duracion_ms;                  // Duraci�n en milisegundos de cada operaci�n
    int tiempo_vida;                 // Tiempo de vida total del hijo en segundos
    int bloques[BLOQUES_POR_HIJO];   // �ndices de bloques asignados al hijo
    time_t tiempo_inicio;            // Momento en que el hijo comienza su ejecuci�n
    time_t tiempo_fin;               // Momento en que el hijo termina su ejecuci�n
} ConfigHijo;

/*
===================================================
      PROTOTIPOS DE FUNCIONES DEFINIDAS EN .C
===================================================
*/

// --- Inicializaci�n de recursos ---
int* crear_memoria_vector();                       // Crea y mapea la memoria compartida
void liberar_memoria_vector(int* vec);             // Libera la memoria compartida

sem_t** crear_mutex_por_bloque();                  // Crea un mutex por cada bloque del vector
void liberar_mutexes(sem_t** mutexes);             // Libera y destruye todos los mutex

void inicializar_vector(int* vec);                 // Rellena el vector con valores aleatorios

// --- Gesti�n de procesos hijos ---
void esperar_y_terminar_hijos(pid_t* hijos, ConfigHijo* configs);  // Supervisa tiempo de vida de hijos
void ejecutar_hijo(int* vec, sem_t** mutexes, int id_hijo, ConfigHijo* configs); // L�gica que ejecuta un hijo

// --- Utilidad ---
void mostrar_vector(const int* vec);               // Muestra el contenido actual del vector por consola


#endif // MAIN_H_INCLUDED
