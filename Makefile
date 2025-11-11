# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LDFLAGS =

# Nome do executável
TARGET = exe

# Arquivos fonte e objetos
SOURCES = cpu.c program.c ram.c
OBJECTS = $(SOURCES:.c=.o)

# Headers (para dependências)
HEADERS = cpu.h ram.h instruction.h opcodes.h

# Regra padrão (compila tudo)
all: $(TARGET)

# Regra para criar o executável
$(TARGET): $(OBJECTS)
	$(CC) -Wall -Wextra $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "✓ Compilação concluída: $(TARGET)"

# Regra para compilar arquivos .c em .o
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpar arquivos compilados
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "✓ Arquivos de compilação removidos"

# Limpar e recompilar tudo
rebuild: clean all

# Executar o programa após compilar
run: $(TARGET)
	./$(TARGET)

# Mostrar informações de debug
debug: CFLAGS += -DDEBUG
debug: rebuild

# Versão otimizada (release)
release: CFLAGS = -Wall -O3 -std=c11
release: rebuild

# Declarar targets que não são arquivos
.PHONY: all clean rebuild run debug release