#include "include/ram.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

RAM* create_ram(size_t size) {
  RAM* ram = malloc(sizeof(RAM));

  ram->memory = (int*)malloc(size * sizeof(int));
  ram->size = size;

  return ram;
}

RAM* create_empty_ram(size_t size) {
  RAM* ram = malloc(sizeof(RAM));

  ram->memory = calloc(size, sizeof(int));
  ram->size = size;

  return ram;
}

RAM* create_random_ram(size_t size) {
  srand(time(NULL));

  RAM* ram = create_ram(size);

  for (size_t i = 0; i < size; i++) {
    ram->memory[i] = rand();
  }

  return ram;
}

void destroy_ram(RAM* ram) {
  free(ram->memory);
  ram->memory = NULL;
  ram->size = 0;

  free(ram);  // free struct
}

void set_ram(RAM* ram, size_t memory_address, int new_memory_value) {
  if (ram == NULL) {
    puts("memory with error");
    exit(1);
  }

  // valida endereço
  if (memory_address >= ram->size) {
    printf("Error: address %zu out of bounds (max: %zu)", memory_address,
           ram->size);
    exit(1);
  }

  ram->memory[memory_address] = new_memory_value;
}

int get_ram(RAM* ram, size_t memory_address) {
  if (ram == NULL) {
    puts("memory with error");
    exit(1);
  }

  // valida endereço
  if (memory_address >= ram->size) {
    printf("Error: address %zu out of bounds (max: %zu)", memory_address,
           ram->size);
    exit(1);
  }

  return ram->memory[memory_address];
}
