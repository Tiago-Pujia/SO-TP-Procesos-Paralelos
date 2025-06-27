#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

/*
===================================================
          INCLUSI�N DE BIBLIOTECAS EST�NDAR
===================================================
*/

#include <stdio.h>      // Funciones de entrada/salida est�ndar (printf, perror, etc.)
#include <stdlib.h>     // Funciones utilitarias (malloc, free, exit, rand, etc.)
#include <string.h>     // Manipulaci�n de cadenas (memset, memcpy, etc.)
#include <fcntl.h>      // Control de archivos (O_CREAT, O_RDWR para shm_open)
#include <sys/mman.h>   // Manejo de memoria compartida (mmap, munmap, shm_open, etc.)
#include <unistd.h>     // Funciones POSIX (fork, close, usleep, etc.)
#include <semaphore.h>  // Sem�foros POSIX para sincronizaci�n de procesos
#include <time.h>       // Medici�n y gesti�n del tiempo (time, srand, etc.)
#include <errno.h>      // Gesti�n de errores del sistema
#include <sys/stat.h>   // Permisos y atributos de archivos
#include <signal.h>     // Manejo de se�ales (SIGTERM, SIGINT, etc.)
#include <sys/wait.h>   // Espera de procesos hijos (wait, waitpid, etc.)

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
    int pid;                            ///< PID del proceso hijo (asignado por fork)
    char operacion;                     ///< C�digo de operaci�n a realizar ('M'ultiplicar, 'P'otenciar, etc.)
    int frecuencia;                     ///< Intervalo (en segundos) entre ejecuciones de operaci�n
    int duracion_ms;                    ///< Duraci�n estimada (en milisegundos) de cada operaci�n
    int tiempo_vida;                    ///< Tiempo total de vida del hijo (en segundos)
    int bloques[BLOQUES_POR_HIJO];      ///< �ndices de bloques asignados en el vector compartido
    time_t tiempo_inicio;               ///< Marca de tiempo al iniciar el proceso hijo
    time_t tiempo_fin;                  ///< Marca de tiempo al finalizar el proceso hijo
} ConfigHijo;

/*
===================================================
      PROTOTIPOS DE FUNCIONES DEFINIDAS EN .C
===================================================
*/

// --- Inicializaci�n de recursos compartidos ---
int* crear_memoria_vector();                        // Crea y mapea el vector en memoria compartida
void liberar_memoria_vector(int* vec);              // Desmapea y elimina el segmento compartido

sem_t** crear_mutex_por_bloque();                   // Crea un sem�foro (mutex) para cada bloque del vector
void liberar_mutexes(sem_t** mutexes);              // Destruye y libera todos los mutex asociados

void inicializar_vector(int* vec);                  // Inicializa el vector con valores aleatorios

// --- Gesti�n de procesos hijos ---
void esperar_y_terminar_hijos(pid_t* hijos, ConfigHijo* configs);
// Espera a que cada hijo finalice y muestra sus estad�sticas

void ejecutar_hijo(int* vec, sem_t** mutexes, int id_hijo, ConfigHijo* configs);
// C�digo ejecutado por cada hijo: realiza operaciones peri�dicas sobre sus bloques

// --- Utilidades ---
void mostrar_vector(const int* vec);
void manejar_senial(int sig);              // Imprime el contenido del vector compartido por consola
void manejador_hijo(int sig);
#endif // MAIN_H_INCLUDED
