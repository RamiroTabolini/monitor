#include "expose_metrics.h"

/** Mutex para sincronización de hilos */
pthread_mutex_t lock;

/** Métrica de Prometheus para el uso de CPU */
static prom_gauge_t* cpu_usage_metric;

/** Métrica de Prometheus para el uso de memoria */
static prom_gauge_t* memory_usage_metric;

/** Métrica de Prometheus para el uso de memoria disponible */
static prom_gauge_t* memory_available_metric;

/** Métrica de Prometheus para la memoria total */
static prom_gauge_t* memory_total_metric;

/** Métricas de Prometheus para las lecturas de disco */
static prom_gauge_t* disk_reads_metric_sda;
static prom_gauge_t* disk_reads_metric_sda1;
static prom_gauge_t* disk_reads_metric_sda2;
static prom_gauge_t* disk_reads_metric_sda3;

/** Métricas de Prometheus para las lecturas de red */
static prom_gauge_t* bytes_received_metric;
static prom_gauge_t* packets_received_metric;
static prom_gauge_t* bytes_transmitted_metric;
static prom_gauge_t* packets_transmitted_metric;

/** Métricas de Prometheus para las lecturas de proc */
prom_gauge_t* process_count_metric;

/** Métricas de Prometheus para las lecturas de ctxt */
prom_gauge_t* context_switches_metric;

void update_cpu_gauge()
{
    double usage = get_cpu_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(cpu_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de CPU\n");
    }
}

void update_memory_gauge()
{
    double usage = get_memory_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de memoria\n");
    }
}

void update_memory_available_gauge()
{
    double available_mem = get_memory_available();
    if (available_mem >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_available_metric, available_mem, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la memoria disponible\n");
    }
}

void update_memory_total_gauge()
{
    double total_mem = get_memory_total();
    if (total_mem >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_total_metric, total_mem, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la memoria total\n");
    }
}

void update_disk_reads_gauge()
{
    double reads;

    // Obtener lecturas para cada dispositivo
    reads = get_disk_reads("sda");
    if (reads >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(disk_reads_metric_sda, (double)reads, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener lecturas de sda\n");
    }

    reads = get_disk_reads("sda1");
    if (reads >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(disk_reads_metric_sda1, (double)reads, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener lecturas de sda1\n");
    }

    reads = get_disk_reads("sda2");
    if (reads >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(disk_reads_metric_sda2, (double)reads, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener lecturas de sda2\n");
    }

    reads = get_disk_reads("sda3");
    if (reads >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(disk_reads_metric_sda3, (double)reads, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener lecturas de sda3\n");
    }
}

void update_network_metrics_gauge()
{
    pthread_mutex_lock(&lock); // Bloqueo

    // Obtener métricas de red
    double bytes_received = get_network_metric("enp0s3", "bytes_received");
    double packets_received = get_network_metric("enp0s3", "packets_received");
    double bytes_transmitted = get_network_metric("enp0s3", "bytes_transmitted");
    double packets_transmitted = get_network_metric("enp0s3", "packets_transmitted");

    // Actualizar las métricas de bytes y paquetes recibidos
    prom_gauge_set(bytes_received_metric, bytes_received, NULL);
    prom_gauge_set(packets_received_metric, packets_received, NULL);

    // Actualizar las métricas de bytes y paquetes transmitidos
    prom_gauge_set(bytes_transmitted_metric, bytes_transmitted, NULL);
    prom_gauge_set(packets_transmitted_metric, packets_transmitted, NULL);

    pthread_mutex_unlock(&lock); // Desbloqueo
}

void update_process_count()
{
    pthread_mutex_lock(&lock);           // Bloqueo
    int count = get_running_processes(); // Llama a la función get
    if (count >= 0)
    {
        const char* labels[] = {NULL};                       // Si no usas etiquetas
        prom_gauge_set(process_count_metric, count, labels); // Actualiza la métrica
    }
    else
    {
        fprintf(stderr, "Error al contar los procesos\n");
    }
    pthread_mutex_unlock(&lock); // Desbloqueo
}

void update_context_switches()
{
    pthread_mutex_lock(&lock);                        // Bloqueo
    double context_switches = get_context_switches(); // Llama a la función que obtiene los cambios de contexto
    if (context_switches >= 0)
    {
        prom_gauge_set(context_switches_metric, context_switches,
                       NULL); // Actualiza la métrica con el valor de cambios de contexto
    }
    else
    {
        fprintf(stderr, "Error al obtener los cambios de contexto\n");
    }
    pthread_mutex_unlock(&lock); // Desbloqueo
}

void* expose_metrics(void* arg)
{
    (void)arg; // Argumento no utilizado

    // Aseguramos que el manejador HTTP esté adjunto al registro por defecto
    promhttp_set_active_collector_registry(NULL);

    // Iniciamos el servidor HTTP en el puerto 8000
    struct MHD_Daemon* daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL);
    if (daemon == NULL)
    {
        fprintf(stderr, "Error al iniciar el servidor HTTP\n");
        return NULL;
    }

    // Mantenemos el servidor en ejecución
    while (1)
    {
        sleep(1);
    }

    // Nunca debería llegar aquí
    MHD_stop_daemon(daemon);
    return NULL;
}

int init_metrics()
{
    printf("Inicializando el mutex...\n");
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "Error al inicializar el mutex\n");
        return -1;
    }
    printf("Mutex inicializado correctamente\n");

    printf("Inicializando el registro de Prometheus...\n");
    if (prom_collector_registry_default_init() != 0)
    {
        fprintf(stderr, "Error al inicializar el registro de Prometheus\n");
        return -1;
    }
    printf("Registro de Prometheus inicializado correctamente\n");

    // Métrica de uso de CPU
    printf("Creando métrica de uso de CPU...\n");
    cpu_usage_metric = prom_gauge_new("cpu_usage_percentage", "Porcentaje de uso de CPU", 0, NULL);
    if (cpu_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de CPU\n");
        return -1;
    }
    printf("Métrica de CPU creada correctamente\n");

    // Métrica de uso de memoria
    printf("Creando métrica de uso de memoria...\n");
    memory_usage_metric = prom_gauge_new("memory_usage_percentage", "Porcentaje de uso de memoria", 0, NULL);
    if (memory_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de memoria\n");
        return -1;
    }
    printf("Métrica de memoria creada correctamente\n");

    printf("Creando métrica de memoria disponible...\n");
    memory_available_metric = prom_gauge_new("memory_available_kilobytes", "Memoria disponible en kilobytes", 0, NULL);
    if (memory_available_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de memoria disponible\n");
        return -1;
    }
    printf("Métrica de memoria disponible creada correctamente\n");

    printf("Creando métrica de memoria total...\n");
    memory_total_metric = prom_gauge_new("memory_total_kb", "Memoria total en kilobytes", 0, NULL);
    if (memory_total_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de memoria total\n");
        return -1;
    }
    printf("Métrica de memoria total creada correctamente\n");

    // Métricas de disco
    printf("Creando métrica de lecturas de disco para sda...\n");
    disk_reads_metric_sda = prom_gauge_new("disk_reads_sda", "Lecturas de disco para sda", 0, NULL);
    if (disk_reads_metric_sda == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de lecturas de disco para sda\n");
        return -1;
    }
    printf("Métrica de lecturas de disco para sda creada correctamente\n");

    printf("Creando métrica de lecturas de disco para sda1...\n");
    disk_reads_metric_sda1 = prom_gauge_new("disk_reads_sda1", "Lecturas de disco para sda1", 0, NULL);
    if (disk_reads_metric_sda1 == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de lecturas de disco para sda1\n");
        return -1;
    }
    printf("Métrica de lecturas de disco para sda1 creada correctamente\n");

    printf("Creando métrica de lecturas de disco para sda2...\n");
    disk_reads_metric_sda2 = prom_gauge_new("disk_reads_sda2", "Lecturas de disco para sda2", 0, NULL);
    if (disk_reads_metric_sda2 == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de lecturas de disco para sda2\n");
        return -1;
    }
    printf("Métrica de lecturas de disco para sda2 creada correctamente\n");

    printf("Creando métrica de lecturas de disco para sda3...\n");
    disk_reads_metric_sda3 = prom_gauge_new("disk_reads_sda3", "Lecturas de disco para sda3", 0, NULL);
    if (disk_reads_metric_sda3 == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de lecturas de disco para sda3\n");
        return -1;
    }
    printf("Métrica de lecturas de disco para sda3 creada correctamente\n");

    // Métricas de red
    printf("Creando métrica de bytes recibidos...\n");
    bytes_received_metric = prom_gauge_new("bytes_received", "Bytes recibidos por la interfaz de red", 0, NULL);
    if (bytes_received_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de bytes recibidos\n");
        return -1;
    }
    printf("Métrica de bytes recibidos creada correctamente\n");

    printf("Creando métrica de paquetes recibidos...\n");
    packets_received_metric = prom_gauge_new("packets_received", "Paquetes recibidos por la interfaz de red", 0, NULL);
    if (packets_received_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de paquetes recibidos\n");
        return -1;
    }
    printf("Métrica de paquetes recibidos creada correctamente\n");

    printf("Creando métrica de bytes transmitidos...\n");
    bytes_transmitted_metric =
        prom_gauge_new("bytes_transmitted", "Bytes transmitidos por la interfaz de red", 0, NULL);
    if (bytes_transmitted_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de bytes transmitidos\n");
        return -1;
    }
    printf("Métrica de bytes transmitidos creada correctamente\n");

    printf("Creando métrica de paquetes transmitidos...\n");
    packets_transmitted_metric =
        prom_gauge_new("packets_transmitted", "Paquetes transmitidos por la interfaz de red", 0, NULL);
    if (packets_transmitted_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de paquetes transmitidos\n");
        return -1;
    }
    printf("Métrica de paquetes transmitidos creada correctamente\n");

    // Registro de las métricas
    printf("Registrando métricas...\n");
    if (prom_collector_registry_must_register_metric(cpu_usage_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de CPU\n");
        return -1;
    }
    if (prom_collector_registry_must_register_metric(memory_usage_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de memoria\n");
        return -1;
    }
    if (prom_collector_registry_must_register_metric(memory_available_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de memoria disponible\n");
        return -1;
    }
    if (prom_collector_registry_must_register_metric(memory_total_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de memoria total\n");
        return -1;
    }
    if (prom_collector_registry_must_register_metric(disk_reads_metric_sda) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de lecturas de disco para sda\n");
        return -1;
    }
    if (prom_collector_registry_must_register_metric(disk_reads_metric_sda1) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de lecturas de disco para sda1\n");
        return -1;
    }
    if (prom_collector_registry_must_register_metric(disk_reads_metric_sda2) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de lecturas de disco para sda2\n");
        return -1;
    }
    if (prom_collector_registry_must_register_metric(disk_reads_metric_sda3) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de lecturas de disco para sda3\n");
        return -1;
    }

    // Registro de métricas de red
    if (prom_collector_registry_must_register_metric(bytes_received_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de bytes recibidos\n");
        return -1;
    }
    if (prom_collector_registry_must_register_metric(packets_received_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de paquetes recibidos\n");
        return -1;
    }
    if (prom_collector_registry_must_register_metric(bytes_transmitted_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de bytes transmitidos\n");
        return -1;
    }
    if (prom_collector_registry_must_register_metric(packets_transmitted_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de paquetes transmitidos\n");
        return -1;
    }

    // Inicializa y registra la métrica de conteo de procesos
    printf("Creando métrica de procesos en ejecucion...\n");
    process_count_metric = prom_gauge_new("process_count", "Numero de procesos en ejecucion", 0, NULL);
    if (prom_collector_registry_must_register_metric(process_count_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de número de procesos\n");
        return -1;
    }
    printf("Métrica de procesos en ejecucion creada correctamente\n");

    // Inicializa la métrica de cambios de contexto
    context_switches_metric = prom_gauge_new("context_switches", "Número de cambios de contexto", 0, NULL);
    if (prom_collector_registry_must_register_metric(context_switches_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de cambios de contexto\n");
        return -1;
    }
    printf("Métrica de cambios de contexto registrada correctamente\n");

    printf("Métricas registradas correctamente\n");
    return 0;
}

void destroy_mutex()
{
    pthread_mutex_destroy(&lock);
}
