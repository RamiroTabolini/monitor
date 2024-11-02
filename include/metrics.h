#ifndef METRICS_H
#define METRICS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 256
#define MAX_DEVICE_NAME_LENGTH 32

double get_memory_usage(void);
double get_memory_available(void);
double get_memory_total(void);
double get_cpu_usage(void);
double get_disk_reads(const char*);
double get_network_metric(const char*, const char*);
int get_running_processes(void);
double get_context_switches(void);

#endif // METRICS_H
