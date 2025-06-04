#include "main.h"

/**
 * Este programa simula un sistema de procesamiento paralelo donde múltiples
 * procesos hijos realizan distintas operaciones sobre un vector de enteros
 * almacenado en memoria compartida. El acceso concurrente está sincronizado
 * mediante semáforos, y el proceso padre coordina la vida útil de cada hijo.
 */
int main()
{
    // ----------------------------------------------------
    // 1. CREACIÓN Y CONFIGURACIÓN DE LOS RECURSOS INICIALES
    // ----------------------------------------------------

    // Crear y mapear memoria compartida para el vector de enteros
    int* vec = crear_memoria_vector();

    // Crear un arreglo de mutexes (uno por bloque)
    sem_t** mutexes = crear_mutex_por_bloque();

    // Inicializar el vector compartido con valores aleatorios
    inicializar_vector(vec);

    // Mostrar el contenido inicial del vector
    printf("\n\t\t----Vector Inicial----\n\n");
    mostrar_vector(vec);

    // ----------------------------------------------------
    // 2. CONFIGURACIÓN DE LOS PROCESOS HIJOS
    // ----------------------------------------------------
    // Arreglo de estructuras que configuran cada proceso hijo.
    // Cada hijo tiene una operación específica y ciertos bloques asignados.

    ConfigHijo hijos_config[NUM_HIJOS] =
    {
        {0, 'M', 3,  80,  60, {0, 3, 6}},  // H1: Buscar máximo cada 3s
        {0, 'P', 5, 120,  80, {1, 4, 7}},  // H2: Calcular promedio cada 5s
        {0, 'O',10, 300, 100, {2, 5, 8}},  // H3: Ordenar valores cada 10s
        {0, 'D', 8, 200, 120, {0, 3, 6}},  // H4: Duplicar valores cada 8s
        {0, 'N', 7, 250,  49, {9, -1, -1}},// H5: Reemplazar negativos cada 7s
        {0, 'I',12, 400,  60, {2, 5, 8}}   // H6: Invertir valores cada 12s
    };

    // Arreglo para guardar los PID de los hijos creados
    pid_t hijos[NUM_HIJOS];

    // ----------------------------------------------------
    // 3. CREACIÓN DE PROCESOS HIJOS
    // ----------------------------------------------------

    for(int i = 0; i < NUM_HIJOS; i++)
    {
        pid_t pid = fork();  // Crear un nuevo proceso hijo

        if(pid < 0)
        {
            // Error al crear el proceso
            perror("fork");

            // Si algún hijo ya fue creado, se finaliza para evitar procesos huérfanos
            for(int j = 0; j < i; j++)
                kill(hijos[j], SIGTERM);

            // Liberar recursos antes de salir
            liberar_mutexes(mutexes);
            liberar_memoria_vector(vec);
            exit(EXIT_FAILURE);
        }
        else if(pid == 0)
        {
            // PROCESO HIJO
            // Ejecuta la función correspondiente a su operación
            ejecutar_hijo(vec, mutexes, i, hijos_config);

            // Termina el proceso hijo cuando finaliza su trabajo
            exit(0);
        }

        // PROCESO PADRE
        // Guarda el PID y marca el tiempo de inicio del hijo
        hijos[i] = pid;
        hijos_config[i].pid = pid;
        hijos_config[i].tiempo_inicio = time(NULL);
    }

    // ----------------------------------------------------
    // 4. ESPERAR Y FINALIZAR PROCESOS HIJOS
    // ----------------------------------------------------

    // El padre espera que cada hijo cumpla su tiempo de vida
    esperar_y_terminar_hijos(hijos, hijos_config);

    // ----------------------------------------------------
    // 5. MOSTRAR RESUMEN DE TIEMPOS
    // ----------------------------------------------------

    printf("\n--- Resumen de Tiempos de Vida de Hijos ---\n");
    for(int i = 0; i < NUM_HIJOS; i++)
    {
        time_t inicio = hijos_config[i].tiempo_inicio;
        time_t fin = hijos_config[i].tiempo_fin;

        // Calcular duración real de vida del proceso hijo
        int duracion_real = (fin != 0 && inicio != 0) ? (int)difftime(fin, inicio) : 0;

        printf("Hijo %d (PID %d) - Operación '%c': Tiempo esperado = %d s, Tiempo real = %d s\n",
               i + 1,
               hijos_config[i].pid,
               hijos_config[i].operacion,
               hijos_config[i].tiempo_vida,
               duracion_real);
    }

    // ----------------------------------------------------
    // 6. MOSTRAR VECTOR FINAL Y LIBERAR RECURSOS
    // ----------------------------------------------------

    // Muestra cómo quedó el vector compartido tras todas las operaciones
    printf("\n\n\t\t----Vector Final----\n\n");
    mostrar_vector(vec);

    // Liberar recursos del sistema (mutexes y memoria)
    liberar_mutexes(mutexes);
    liberar_memoria_vector(vec);

    return 0;
}
