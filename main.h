#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

/*
===================================================
          INCLUSIÓN DE BIBLIOTECAS ESTÁNDAR
===================================================
*/

#include <stdio.h>      // Para funciones de entrada/salida como printf, perror, etc.
#include <stdlib.h>     // Para funciones como malloc, free, exit, rand, etc.
#include <string.h>     // Para funciones de manipulación de cadenas
#include <fcntl.h>      // Para constantes como O_CREAT, O_RDWR usadas en shm_open
#include <sys/mman.h>   // Para funciones de mapeo de memoria compartida (mmap, munmap)
#include <unistd.h>     // Para funciones como close, usleep, fork, etc.
#include <semaphore.h>  // Para manejo de semáforos POSIX
#include <time.h>       // Para manejo de tiempo y fechas (time, time_t, srand, etc.)
#include <errno.h>      // Para la gestión de errores del sistema
#include <sys/stat.h>   // Para permisos de archivos y memoria compartida
#include <signal.h>     // Para manejo de señales como SIGTERM
#include <sys/wait.h>   // Para espera de procesos hijos (waitpid)

/*
===================================================
                   DEFINICIONES
===================================================
*/

#define NUM_BLOQUES 10          // Número total de bloques en el vector compartido
#define TAM_VECTOR 100          // Tamaño total del vector de enteros compartido
#define NUM_HIJOS 6             // Número total de procesos hijos que se crearán
#define BLOQUES_POR_HIJO 3      // Cantidad de bloques que gestiona cada hijo

/*
===================================================
     ESTRUCTURA DE CONFIGURACIÓN DEL HIJO
===================================================
*/

/**
 * Estructura que define la configuración individual de un proceso hijo.
 */
typedef struct
{
    int pid;                            // PID del proceso hijo (usado para seguimiento)
    char operacion;                    // Código de operación a ejecutar (e.g. 'M', 'P', etc.)
    int frecuencia;                    // Frecuencia en segundos entre ciclos de trabajo
    int duracion_ms;                  // Duración en milisegundos de cada operación
    int tiempo_vida;                 // Tiempo de vida total del hijo en segundos
    int bloques[BLOQUES_POR_HIJO];   // Índices de bloques asignados al hijo
    time_t tiempo_inicio;            // Momento en que el hijo comienza su ejecución
    time_t tiempo_fin;               // Momento en que el hijo termina su ejecución
} ConfigHijo;

/*
===================================================
      PROTOTIPOS DE FUNCIONES DEFINIDAS EN .C
===================================================
*/

// --- Inicialización de recursos ---
int* crear_memoria_vector();                       // Crea y mapea la memoria compartida
void liberar_memoria_vector(int* vec);             // Libera la memoria compartida

sem_t** crear_mutex_por_bloque();                  // Crea un mutex por cada bloque del vector
void liberar_mutexes(sem_t** mutexes);             // Libera y destruye todos los mutex

void inicializar_vector(int* vec);                 // Rellena el vector con valores aleatorios

// --- Gestión de procesos hijos ---
void esperar_y_terminar_hijos(pid_t* hijos, ConfigHijo* configs);  // Supervisa tiempo de vida de hijos
void ejecutar_hijo(int* vec, sem_t** mutexes, int id_hijo, ConfigHijo* configs); // Lógica que ejecuta un hijo

// --- Utilidad ---
void mostrar_vector(const int* vec);               // Muestra el contenido actual del vector por consola


#endif // MAIN_H_INCLUDED
