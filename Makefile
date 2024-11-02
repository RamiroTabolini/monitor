# Variables
CC = gcc
CFLAGS = -Wall -g -Iinclude -I~/.conan2/p/cjsona7e0d0d98076a/s/src
# -Ilib/prom/include -Ilib/prom/src -Ilib/promhttp/include
SRC = src/main.c src/expose_metrics.c src/metrics.c
OBJ = $(SRC:.c=.o)
TARGET = TP1-RAMIRO

# Librerías
LIBS = -Llib -lpthread -lprom -lpromhttp -lmicrohttpd -L~/.conan2/p/cjsona7e0d0d98076  # -Llib indica la carpeta donde buscar las bibliotecas

# Regla principal
all: $(TARGET)

# Regla para construir el ejecutable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

# Regla para compilar archivos .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f $(OBJ) $(TARGET)