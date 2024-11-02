#include "metrics.h"

double get_memory_usage()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long total_mem = 0, free_mem = 0;

    // Abrir el archivo /proc/meminfo
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/meminfo");
        return -1.0;
    }

    // Leer los valores de memoria total y disponible
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "MemTotal: %llu kB", &total_mem) == 1)
        {
            continue; // MemTotal encontrado
        }
        if (sscanf(buffer, "MemAvailable: %llu kB", &free_mem) == 1)
        {
            break; // MemAvailable encontrado, podemos dejar de leer
        }
    }

    fclose(fp);

    // Verificar si se encontraron ambos valores
    if (total_mem == 0 || free_mem == 0)
    {
        fprintf(stderr, "Error al leer la información de memoria desde /proc/meminfo\n");
        return -1.0;
    }

    // Calcular el porcentaje de uso de memoria
    double used_mem = total_mem - free_mem;
    double mem_usage_percent = (used_mem / total_mem) * 100.0;

    return mem_usage_percent;
}

double get_memory_available()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long available_mem = 0;

    // Abrir el archivo /proc/meminfo
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/meminfo");
        return -1.0;
    }

    // Leer el valor de MemAvailable
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "MemAvailable: %llu kB", &available_mem) == 1)
        {
            break; // MemAvailable encontrado
        }
    }

    fclose(fp);

    // Verificar si se encontró el valor
    if (available_mem == 0)
    {
        fprintf(stderr, "Error al leer MemAvailable desde /proc/meminfo\n");
        return -1.0;
    }

    // Devolver la memoria disponible en kilobytes
    return (double)available_mem;
}

double get_memory_total()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long total_mem = 0;

    // Abrir el archivo /proc/meminfo
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/meminfo");
        return -1.0;
    }

    // Leer el valor de MemTotal
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "MemTotal: %llu kB", &total_mem) == 1)
        {
            break; // MemTotal encontrado
        }
    }

    fclose(fp);

    // Verificar si se encontró el valor
    if (total_mem == 0)
    {
        fprintf(stderr, "Error al leer MemTotal desde /proc/meminfo\n");
        return -1.0;
    }

    // Devolver la memoria total en kilobytes
    return (double)total_mem;
}

double get_cpu_usage()
{
    static unsigned long long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0, prev_iowait = 0,
                              prev_irq = 0, prev_softirq = 0, prev_steal = 0;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    unsigned long long totald, idled;
    double cpu_usage_percent;

    // Abrir el archivo /proc/stat
    FILE* fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/stat");
        return -1.0;
    }

    char buffer[BUFFER_SIZE * 4];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error al leer /proc/stat");
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    // Analizar los valores de tiempo de CPU
    int ret = sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait,
                     &irq, &softirq, &steal);
    if (ret < 8)
    {
        fprintf(stderr, "Error al parsear /proc/stat\n");
        return -1.0;
    }

    // Calcular las diferencias entre las lecturas actuales y anteriores
    unsigned long long prev_idle_total = prev_idle + prev_iowait;
    unsigned long long idle_total = idle + iowait;

    unsigned long long prev_non_idle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;
    unsigned long long non_idle = user + nice + system + irq + softirq + steal;

    unsigned long long prev_total = prev_idle_total + prev_non_idle;
    unsigned long long total = idle_total + non_idle;

    totald = total - prev_total;
    idled = idle_total - prev_idle_total;

    if (totald == 0)
    {
        fprintf(stderr, "Totald es cero, no se puede calcular el uso de CPU!\n");
        return -1.0;
    }

    // Calcular el porcentaje de uso de CPU
    cpu_usage_percent = ((double)(totald - idled) / totald) * 100.0;

    // Actualizar los valores anteriores para la siguiente lectura
    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;
    prev_iowait = iowait;
    prev_irq = irq;
    prev_softirq = softirq;
    prev_steal = steal;

    return cpu_usage_percent;
}

double get_disk_reads(const char* device_name)
{
    FILE* file = fopen("/proc/diskstats", "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error al abrir /proc/diskstats\n");
        return -1.0; // Devuelve -1.0 en caso de error
    }

    char line[BUFFER_SIZE];
    unsigned long reads = 0;

    while (fgets(line, sizeof(line), file))
    {
        char device[MAX_DEVICE_NAME_LENGTH]; // Asegúrate de que el tamaño sea suficiente

        // Extraer el dispositivo y las lecturas de la línea
        if (sscanf(line, "%*d %*d %s %lu", device, &reads) == 2)
        { // Limitar el tamaño de entrada
            if (strcmp(device, device_name) == 0)
            {
                fclose(file);
                return (double)reads; // Devuelve las lecturas como double
            }
        }
    }

    fclose(file);
    return -1.0; // Devuelve -1.0 si no encontró el dispositivo
}

double get_network_metric(const char* interface_name, const char* metric_type)
{
    FILE* file = fopen("/proc/net/dev", "r");
    if (!file)
    {
        perror("fopen");
        return -1; // Devuelve -1 en caso de error
    }

    char line[BUFFER_SIZE];
    double value = 0.0;

    while (fgets(line, sizeof(line), file))
    {
        // Buscamos la interfaz de red específica
        if (strstr(line, interface_name) != NULL)
        {
            double bytes_received, packets_received, bytes_transmitted, packets_transmitted;
            sscanf(line, "%*s %lf %lf %*d %*d %*d %*d %*d %*d %lf %lf %*d %*d %*d %*d %*d", &bytes_received,
                   &packets_received, &bytes_transmitted, &packets_transmitted);

            if (strcmp(metric_type, "bytes_received") == 0)
            {
                value = bytes_received;
            }
            else if (strcmp(metric_type, "packets_received") == 0)
            {
                value = packets_received;
            }
            else if (strcmp(metric_type, "bytes_transmitted") == 0)
            {
                value = bytes_transmitted;
            }
            else if (strcmp(metric_type, "packets_transmitted") == 0)
            {
                value = packets_transmitted;
            }
            break; // Salimos una vez que encontramos la interfaz
        }
    }

    fclose(file);
    return value; // Devolvemos el valor obtenido
}

int get_running_processes()
{
    FILE* file;
    char line[BUFFER_SIZE];
    int procs_running = -1; // Valor por defecto en caso de error

    // Abre el archivo /proc/stat
    file = fopen("/proc/stat", "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1; // Manejo de error
    }

    // Lee cada línea en el archivo
    while (fgets(line, sizeof(line), file))
    {
        // Busca la línea que comienza con "procs_running"
        if (strncmp(line, "procs_running", 13) == 0)
        {
            // Extrae el número de procesos
            sscanf(line + 14, "%d", &procs_running); // Lee el número
            break;                                   // Sale del bucle después de encontrarlo
        }
    }

    fclose(file);
    return procs_running; // Retorna el número de procesos en ejecución
}

double get_context_switches()
{
    FILE* file;
    char line[BUFFER_SIZE];
    unsigned long long ctxt = 0;

    // Abre el archivo /proc/stat
    file = fopen("/proc/stat", "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1.0; // Retorna -1.0 para manejar el error como double
    }

    // Lee cada línea en el archivo
    while (fgets(line, sizeof(line), file))
    {
        // Busca la línea que comienza con "ctxt"
        if (strncmp(line, "ctxt", 4) == 0)
        {
            // Extrae el número de cambios de contexto
            sscanf(line + 5, "%llu", &ctxt); // Lee el número
            break;                           // Sale del bucle después de encontrarlo
        }
    }

    fclose(file);
    return (double)ctxt; // Retorna el número de cambios de contexto como double
}
