#include "main.h"

/*
==========================================
GESTI�N DE MEMORIA COMPARTIDA
==========================================
*/

/**
 * Crea un segmento de memoria compartida llamado "/memvec", lo configura
 * con el tama�o adecuado para un vector de enteros, y lo mapea al espacio
 * de direcciones del proceso.
 *
 * @return Puntero a la memoria compartida (cast a int*).
 */
int* crear_memoria_vector()
{
    // Crea (o abre si ya existe) un objeto de memoria compartida con permisos de lectura/escritura
    int fd = shm_open("/memvec", O_CREAT | O_RDWR, 0666);
    if(fd == -1)
    {
        perror("shm_open"); // Muestra mensaje de error si no se pudo crear/abrir
        exit(EXIT_FAILURE); // Finaliza el programa
    }

    // Define el tama�o de la memoria compartida para alojar TAM_VECTOR enteros
    if(ftruncate(fd, sizeof(int) * TAM_VECTOR) == -1)
    {
        perror("ftruncate"); // Error al establecer el tama�o
        close(fd); // Cierra el descriptor
        shm_unlink("/memvec"); // Elimina el segmento creado
        exit(EXIT_FAILURE);
    }

    // Mapea el segmento de memoria en el espacio del proceso con permisos de lectura y escritura, compartido entre procesos
    void* ptr = mmap(NULL, sizeof(int) * TAM_VECTOR, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd); // Ya no se necesita el descriptor una vez hecho el mapeo

    if(ptr == MAP_FAILED)
    {
        perror("mmap"); // Falla en el mapeo
        shm_unlink("/memvec"); // Elimina el segmento compartido
        exit(EXIT_FAILURE);
    }

    return (int*) ptr; // Devuelve puntero a la memoria compartida
}

/**
 * Libera los recursos asociados a la memoria compartida.
 *
 * @param vec Puntero al vector en memoria compartida.
 */
void liberar_memoria_vector(int* vec)
{
    // Desmapea la memoria del espacio de direcciones del proceso
    if(munmap(vec, sizeof(int) * TAM_VECTOR) == -1)
        perror("munmap");

    // Elimina el objeto de memoria compartida del sistema
    if(shm_unlink("/memvec") == -1)
        perror("shm_unlink");
}

/*
==========================================
GESTI�N DE SEM�FOROS MUTEX
==========================================
*/

/**
 * Crea un conjunto de sem�foros con nombre (uno por bloque) para proteger
 * regiones del vector compartido.
 *
 * @return Arreglo de punteros a sem�foros.
 */
sem_t** crear_mutex_por_bloque()
{
    // Reserva memoria din�mica para el arreglo de punteros a sem�foros
    sem_t** sems = malloc(NUM_BLOQUES * sizeof(sem_t*));
    if(!sems)
    {
        perror("malloc"); // Falla en la reserva de memoria
        exit(EXIT_FAILURE);
    }

    char nombre[20]; // Buffer temporal para los nombres de sem�foros

    for(int i = 0; i < NUM_BLOQUES; i++)
    {
        // Genera un nombre �nico para cada sem�foro (e.g. /mutex_0, /mutex_1, ...)
        snprintf(nombre, sizeof(nombre), "/mutex_%d", i);

        // Crea o abre un sem�foro con nombre, inicializado en 1 (mutex)
        sems[i] = sem_open(nombre, O_CREAT, 0666, 1);
        if(sems[i] == SEM_FAILED)
        {
            perror("sem_open"); // Falla al crear/abrir
            for (int j = 0; j < i; j++) sem_close(sems[j]); // Cierra sem�foros ya abiertos
            free(sems); // Libera memoria del arreglo
            exit(EXIT_FAILURE);
        }
    }

    return sems;
}

/**
 * Libera los recursos asociados a los sem�foros creados.
 *
 * @param mutexes Arreglo de punteros a sem�foros.
 */
void liberar_mutexes(sem_t** mutexes)
{
    char nombre[20];
    for(int i = 0; i < NUM_BLOQUES; i++)
    {
        // Cierra el sem�foro asociado
        sem_close(mutexes[i]);

        // Genera el nombre nuevamente para eliminarlo del sistema
        snprintf(nombre, sizeof(nombre), "/mutex_%d", i);
        sem_unlink(nombre);
    }

    // Libera la memoria usada para almacenar los punteros a sem�foros
    free(mutexes);
}

/**
 * Inicializa el vector compartido con valores aleatorios entre -50 y 50.
 *
 * @param vec Vector compartido a inicializar.
 */
void inicializar_vector(int* vec)
{
    srand(time(NULL)); // Inicializa la semilla aleatoria con el tiempo actual
    for(int i = 0; i < TAM_VECTOR; i++)
        vec[i] = (rand() % 101) - 50; // Rango [-50, 50]
}

/*
==========================================
GESTI�N DE HIJOS Y TIEMPOS DE VIDA
==========================================
*/

/**
 * Espera a que transcurra el tiempo m�ximo o que los hijos excedan su
 * tiempo de vida, y los termina si corresponde.
 *
 * @param hijos Arreglo de PIDs de los procesos hijos.
 * @param configs Arreglo de estructuras de configuraci�n de los hijos.
 */
void esperar_y_terminar_hijos(pid_t* hijos, ConfigHijo* configs)
{
    int tiempo = 0;

    // Bucle que corre hasta que se alcance el l�mite de tiempo (130 segundos)
    while(tiempo <= 130)
    {
        sleep(1); // Espera un segundo
        tiempo++;

        // Verifica si alg�n hijo super� su tiempo de vida
        for(int i = 0; i < NUM_HIJOS; i++)
        {
            if(hijos[i] != 0 && tiempo >= configs[i].tiempo_vida)
            {
                kill(hijos[i], SIGTERM); // Termina el hijo
                printf("Padre termino al hijo %d (PID %d)\n", i + 1, hijos[i]);
                configs[i].tiempo_fin = time(NULL); // Registra el tiempo de finalizaci�n
                hijos[i] = 0; // Marca como terminado
            }
        }
    }

    // Espera a que terminen los procesos hijos restantes
    for(int i = 0; i < NUM_HIJOS; i++)
        if (hijos[i])
        {
            waitpid(hijos[i], NULL, 0);
            configs[i].tiempo_fin = time(NULL);
        }
}

/**
 * Funci�n que ejecuta un proceso hijo con base en su configuraci�n.
 * Cada hijo accede a ciertos bloques del vector y realiza una operaci�n.
 *
 * @param vec Vector compartido.
 * @param mutexes Arreglo de mutexes por bloque.
 * @param id_hijo �ndice del hijo que se est� ejecutando.
 * @param configs Arreglo de configuraciones.
 */
void ejecutar_hijo(int* vec, sem_t** mutexes, int id_hijo, ConfigHijo* configs)
{
    ConfigHijo cfg = configs[id_hijo];
    int tiempo = 0;

    while(tiempo < cfg.tiempo_vida)
    {
        for(int i = 0; i < BLOQUES_POR_HIJO; i++)
        {
            int bloque = cfg.bloques[i];
            if (bloque < 0 || bloque >= NUM_BLOQUES) continue; // Valida el �ndice

            sem_wait(mutexes[bloque]); // Entra a la regi�n cr�tica

            int inicio = bloque * (TAM_VECTOR / NUM_BLOQUES); // �ndice de inicio del bloque
            int fin = inicio + (TAM_VECTOR / NUM_BLOQUES);    // �ndice de fin (no inclusivo)

            // Ejecuta la operaci�n especificada por la configuraci�n
            switch(cfg.operacion)
            {
                case 'M': // M�ximo del bloque
                    {
                        int max = vec[inicio];
                        for(int j = inicio + 1; j < fin; j++)
                            if(vec[j] > max) max = vec[j];
                        printf("[Hijo %d] Maximo bloque %d: %d\n", getpid(), bloque, max);
                        fflush(stdout);
                        break;
                    }
                case 'P': // Promedio del bloque
                    {
                        int suma = 0;
                        for(int j = inicio; j < fin; j++) suma += vec[j];
                        float prom = suma / (float)(fin - inicio);
                        printf("[Hijo %d] Promedio bloque %d: %.2f\n", getpid(), bloque, prom);
                        fflush(stdout);
                        break;
                    }
                case 'O': // Ordenar el bloque (burbuja)
                    {
                        for(int j = inicio; j < fin - 1; j++)
                            for(int k = j + 1; k < fin; k++)
                                if(vec[j] > vec[k])
                                {
                                    int tmp = vec[j];
                                    vec[j] = vec[k];
                                    vec[k] = tmp;
                                }
                        printf("[Hijo %d] Ordeno bloque %d\n", getpid(), bloque);
                        fflush(stdout);
                        break;
                    }
                case 'D': // Duplicar valores
                    {
                        for(int j = inicio; j < fin; j++) vec[j] *= 2;
                        printf("[Hijo %d] Duplico bloque %d\n", getpid(), bloque);
                        fflush(stdout);
                        break;
                    }
                case 'N': // Eliminar negativos (reemplazar por 0)
                    {
                        for(int j = inicio; j < fin; j++)
                            if(vec[j] < 0) vec[j] = 0;
                        printf("[Hijo %d] Reemplazo negativos en bloque %d\n", getpid(), bloque);
                        fflush(stdout);
                        break;
                    }
                case 'I': // Invertir el orden del bloque
                    {
                        for(int j = 0; j < (fin - inicio) / 2; j++)
                        {
                            int tmp = vec[inicio + j];
                            vec[inicio + j] = vec[fin - 1 - j];
                            vec[fin - 1 - j] = tmp;
                        }
                        printf("[Hijo %d] Invirtio bloque %d\n", getpid(), bloque);
                        fflush(stdout);
                        break;
                    }
            }

            sem_post(mutexes[bloque]); // Sale de la regi�n cr�tica

            // Espera configurada antes de pasar al siguiente bloque
            usleep(cfg.duracion_ms * 1000);
        }

        // Espera antes del siguiente ciclo de procesamiento
        sleep(cfg.frecuencia);
        tiempo += cfg.frecuencia;
    }
}

/**
 * Muestra el contenido actual del vector compartido en consola.
 *
 * @param vec Vector a mostrar.
 */
void mostrar_vector(const int* vec)
{
    for(int i = 0; i < TAM_VECTOR; i++)
    {
        printf("%8d", vec[i]);
        if ((i + 1) % 10 == 0) printf("\n"); // Salto de l�nea cada 10 elementos
    }
    printf("\n");
}
//MANEJAR LAS SENIALES
void manejar_senial(int sig) {
    printf("\n[Padre] Se�al %d recibida. Finalizando...\n", sig);

    // Matar a los hijos
    for (int i = 0; i < NUM_HIJOS; i++) {
        if (g_hijos_config[i].pid > 0) {
            kill(g_hijos_config[i].pid, SIGTERM);
        }
    }

    // Liberar recursos compartidos
    if (g_mutexes != NULL)
        liberar_mutexes(g_mutexes);
    if (g_vec != NULL)
        liberar_memoria_vector(g_vec);

    exit(0);
}
