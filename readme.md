# 🧠 Proyecto: Procesamiento Concurrente sobre Memoria Compartida

## 🎯 Objetivo General

Este proyecto simula un entorno de **procesamiento concurrente** donde múltiples procesos hijos (workers) realizan distintas operaciones sobre un vector de enteros en **memoria compartida**. La sincronización se realiza mediante **semáforos (mutexes por bloque)** para evitar condiciones de carrera, asegurando que cada proceso trabaje de forma segura y controlada.

---

## ⚙️ Componentes del Sistema

### 🔹 1. Memoria Compartida

- Se reserva un vector de tamaño fijo (`TAM_VECTOR = 100`) usando `mmap`.
- Este vector se divide en **10 bloques** (`NUM_BLOQUES = 10`), cada uno con 10 elementos.
- Cada proceso hijo trabaja con **3 bloques específicos**.

### 🔹 2. Semáforos por Bloque

- Se crea un **mutex (semáforo POSIX)** por cada bloque.
- Permite sincronización a nivel de bloque para prevenir accesos simultáneos conflictivos.

### 🔹 3. Configuración de Procesos Hijos

- Se lanzan **6 procesos hijos** (`NUM_HIJOS = 6`), cada uno con:
  - Tipo de operación (`M`, `P`, `O`, `D`, `N`, `I`).
  - Frecuencia de ejecución (en segundos).
  - Duración estimada de su operación (en milisegundos).
  - Tiempo total de vida (en segundos).
  - 3 bloques asignados del vector.

### 🔹 4. Operaciones Implementadas

Cada hijo ejecuta periódicamente una operación distinta sobre sus bloques:

| Código | Operación             | Descripción                                              |
|--------|------------------------|----------------------------------------------------------|
| `M`    | Máximo                 | Calcula y muestra el valor máximo de sus bloques.        |
| `P`    | Promedio               | Calcula y muestra el promedio de sus bloques.            |
| `O`    | Ordenar                | Ordena los valores de sus bloques.                       |
| `D`    | Duplicar               | Duplica cada valor de sus bloques.                       |
| `N`    | Reemplazar negativos   | Reemplaza valores negativos por su valor absoluto.       |
| `I`    | Invertir               | Invierte el orden de los valores en sus bloques.         |

---

## 🔁 Ciclo de Vida del Sistema

1. **Inicialización**
   - Se crean la memoria compartida y los mutexes.
   - Se inicializa el vector con valores aleatorios.

2. **Ejecución Concurrente**
   - Se crean 6 procesos hijos con `fork()`.
   - Cada hijo ejecuta periódicamente su operación en los bloques asignados.
   - Se usan mutexes para garantizar sincronización a nivel de bloque.

3. **Finalización**
   - El proceso padre espera a que cada hijo complete su tiempo de vida.
   - Se muestra un resumen del tiempo real vivido por cada hijo.
   - Se libera la memoria compartida y los semáforos.

---

## 🧩 Estructura de Archivos

| Archivo         | Descripción                                                                 |
|-----------------|------------------------------------------------------------------------------|
| `main.c`        | Función principal. Inicializa todo y gestiona la creación y finalización de hijos. |
| `main.h`        | Definiciones, constantes, estructuras y prototipos de funciones.             |
| `herramientas.c`| Funciones auxiliares para la memoria compartida, semáforos y vector.         |
| `operaciones.c` | Implementación de las 6 operaciones posibles.                                |
| `hijo.c`        | Implementa la lógica de los hijos: ejecución periódica de operaciones.       |

---

## 🧪 Aspectos Técnicos

- 🧵 **Procesamiento concurrente multiproceso** usando `fork()`.
- 🧠 **Memoria compartida anónima** (`mmap`) sin archivos intermedios.
- 🛡️ **Sincronización con semáforos POSIX** (`sem_t`).
- ⏱️ **Control de tiempo** con `time()` y `usleep()`.