# 🧠 Proyecto: Procesamiento Concurrente sobre Memoria Compartida

## 🎯 Objetivo General

Este proyecto simula un entorno de **procesamiento concurrente** donde múltiples procesos hijos (workers) realizan distintas operaciones sobre un vector de enteros en **memoria compartida**. La sincronización se realiza mediante **semáforos (mutexes por bloque)** para evitar condiciones de carrera, asegurando que cada proceso trabaje de forma segura y controlada.

## 🔧 Tecnologías y herramientas

-   Lenguaje: C (ISO C99)
-   Plataforma: Linux Ubuntu
-   Herramientas:
    -   `gcc` para compilación
    -   `shm_open`, `mmap` para memoria compartida
    -   `sem_open`, `sem_wait`, sem_post para semáforos
    -   `fork`, `kill`, `waitpid` para gestión de procesos
    -   Comandos de monitoreo: `ipcs`, `ps`, `htop`, `vmstat`

## 🗂️ Archivos del Proyecto

```
.
├── main.c             // Función principal
├── main.h             // Definiciones y prototipos
├── funciones.c        // Funciones
├── README.md          // Este archivo
```

## ⚙️ Compilación

```
gcc -Wall -o sistema main.c -lrt -pthread
```

## 🔄 Flujo de vida del sistema

### 🧩 1. Inicio del programa (main)

El proceso principal (padre) comienza su ejecución.

-   Se inicializan los recursos compartidos:
    -   Se crea y mapea un vector de enteros en memoria compartida usando shm_open y mmap.
    -   Se crean semáforos POSIX con nombre, uno por cada bloque del vector, para sincronizar el acceso entre procesos.
    -   El vector se rellena con valores aleatorios en el rango [-10, 9].

### 👶 2. Creación de procesos hijos (fork)

-   Se crean NUM_HIJOS procesos hijos con fork().
    -   Cada hijo recibe una configuración individual:
    -   Código de operación
        | Código | Operación | Descripción |
        |--------|------------------------|----------------------------------------------------------|
        | `M` | Máximo | Calcula y muestra el valor máximo de sus bloques. |
        | `P` | Promedio | Calcula y muestra el promedio de sus bloques. |
        | `O` | Ordenar | Ordena los valores de sus bloques. |
        | `D` | Duplicar | Duplica cada valor de sus bloques. |
        | `N` | Reemplazar negativos | Reemplaza valores negativos por su valor absoluto. |
        | `I` | Invertir | Invierte el orden de los valores en sus bloques. |
    -   Frecuencia con la que trabaja (cada X segundos).
    -   Duración de cada tarea (en milisegundos).
    -   Tiempo de vida (cuánto tiempo estará activo).
    -   Qué bloques del vector manipula.
-   Cada hijo ejecuta la función ejecutar_hijo.

### 🧠 3. Ejecución de los procesos hijos

Cada hijo entra en un ciclo de vida con este patrón:

1. Espera frecuencia segundos.
2. Por cada uno de sus bloques asignados:
    - Espera el semáforo correspondiente (sem_wait).
    - Realiza su operación (máximo, promedio, ordenar, etc.) sobre el bloque.
    - Libera el semáforo (sem_post).
    - Espera duración_ms milisegundos.
3. Se repite hasta cumplir su tiempo_vida.

### 🧑‍✈️ 4. Supervisión del padre

Mientras los hijos trabajan, el proceso padre:

-   Lleva un contador de tiempo (hasta 130 segundos).
-   Controla el tiempo de vida de cada hijo:
    -   Si un hijo superó su tiempo de vida → kill(SIGTERM).
    -   Registra el tiempo de finalización.
-   Al final, espera que todos los hijos terminen (waitpid).

### 🧼 5. Finalización y limpieza

-   Se imprime un resumen de la vida de cada hijo (esperado vs real).
-   Se imprime el vector final luego de todas las modificaciones.
-   Se liberan todos los recursos del sistema:
    -   Memoria compartida (munmap + shm_unlink)
    -   Semáforos (sem_close + sem_unlink)