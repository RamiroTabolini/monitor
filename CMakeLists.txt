# CMakeLists.txt en la carpeta monitor

cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto del monitor
project(Monitor C)

# Establecemos el estándar de C
set(CMAKE_C_STANDARD 17)

# Incluir directorios de encabezado
include_directories(include)

# Archivos fuente del monitor
set(MONITOR_SRC src/expose_metrics.c src/metrics.c src/main.c)

# Crear el ejecutable del monitor
add_executable(monitor ${MONITOR_SRC})

# Especificar bibliotecas necesarias
target_link_libraries(monitor PRIVATE pthread prom promhttp cjson::cjson microhttpd)


