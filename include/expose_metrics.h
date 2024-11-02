// expose_metrics.h

#ifndef EXPOSE_METRICS_H
#define EXPOSE_METRICS_H

#include "metrics.h"
// #include "read_cpu_usage.h"
#include <errno.h>
#include <prom.h>
#include <promhttp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Para sleep

#define BUFFER_SIZE 256

// Prototipos de las funciones
void update_cpu_gauge(void);
void update_memory_gauge(void);
void update_memory_available_gauge(void);
void update_memory_total_gauge(void);
void update_disk_reads_gauge(void);
void update_network_metrics_gauge(void);
void update_process_count(void);
void update_context_switches(void);
void* expose_metrics(void*);
int init_metrics(void);
void destroy_mutex(void);

#endif // EXPOSE_METRICS_H