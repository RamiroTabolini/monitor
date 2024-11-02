#include "expose_metrics.h"
#include <cjson/cJSON.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_INTERVAL 1

void cargar_config(void);

int interval = DEFAULT_INTERVAL;
bool monitor_cpu = true;
bool monitor_memory = true;
bool monitor_disk = true;
bool monitor_network = true;

void cargar_config()
{
    /// home/ramiro/Escritorio/TP2/so-i-24-chp2-RamiroTabolini/monitor/src/
    FILE* file = fopen("../monitor/src/config.json", "r");
    if (!file)
    {
        fprintf(stderr, "No se pudo abrir config.json, usando configuración predeterminada\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = (char*)malloc(length + 1);
    if (!data)
    {
        fprintf(stderr, "Error al asignar memoria para leer config.json\n");
        fclose(file);
        return;
    }

    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0'; // Asegurarse de que la cadena esté terminada en null

    cJSON* json = cJSON_Parse(data);
    free(data); // Liberar la memoria después de parsear

    if (!json)
    {
        const char* error_ptr = cJSON_GetErrorPtr();
        fprintf(stderr, "Error al parsear config.json: %s\n", error_ptr ? error_ptr : "Error desconocido\n");
        return;
    }

    // Leer intervalo
    cJSON* interval_json = cJSON_GetObjectItem(json, "interval");
    if (cJSON_IsNumber(interval_json))
    {
        interval = interval_json->valueint;
        printf("Intervalo configurado a %d\n", interval);
    }
    else
    {
        fprintf(stderr, "El intervalo no es un número, usando configuración predeterminada\n");
    }

    // Leer métricas
    cJSON* metrics = cJSON_GetObjectItem(json, "metrics");
    if (metrics)
    {
        monitor_cpu = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "cpu_usage"));
        monitor_memory = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "memory_usage"));
        monitor_disk = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "disk_reads"));
        monitor_network = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "network_traffic"));

        printf("Configuración de métricas:\n");
        printf("  CPU Usage: %s\n", monitor_cpu ? "Activado" : "Desactivado");
        printf("  Memory Usage: %s\n", monitor_memory ? "Activado" : "Desactivado");
        printf("  Disk Reads: %s\n", monitor_disk ? "Activado" : "Desactivado");
        printf("  Network Traffic: %s\n", monitor_network ? "Activado" : "Desactivado");
    }

    cJSON_Delete(json);
}

int main(void)
{
    cargar_config();

    if (init_metrics() != 0)
    {
        fprintf(stderr, "Error al inicializar las métricas\n");
        return EXIT_FAILURE;
    }

    pthread_t tid;
    if (pthread_create(&tid, NULL, expose_metrics, NULL) != 0)
    {
        fprintf(stderr, "Error al crear el hilo del servidor HTTP\n");
        return EXIT_FAILURE;
    }

    while (true)
    {
        if (monitor_cpu)
            update_cpu_gauge();
        if (monitor_memory)
        {
            update_memory_gauge();
            update_memory_available_gauge();
            update_memory_total_gauge();
        }
        if (monitor_disk)
            update_disk_reads_gauge();
        if (monitor_network)
            update_network_metrics_gauge();

        sleep(interval);
    }

    return EXIT_SUCCESS;
}