CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -Iinclude -I.
LDFLAGS =

TARGET = bin/exe
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "✓ Compilação concluída: $(TARGET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) bin
	@echo "✓ Limpeza concluída"

re: clean all

run: $(TARGET)
	./$(TARGET)

. PHONY: all clean rebuild run
