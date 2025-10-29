#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ram.h"

RAM* create_ram(size_t size) {
  RAM *ram = malloc(sizeof(RAM));

  ram->memory = (int *) malloc(size * sizeof(int));
  ram->size = size;

  return ram;
}

RAM* create_empty_ram(size_t size) {
  RAM *ram = malloc(sizeof(RAM));

  ram->memory = calloc(size, sizeof(RAM));
  ram->size = size;

  return ram;
}

RAM* create_random_ram(size_t size) {
  RAM *ram = create_ram(size);

  for(size_t i = 0; i < size; i++) {
    ram->memory[i] = rand();
  }

  return ram;
}

void destroy_ram(RAM *ram) {
  free(ram->memory);
  ram->memory = NULL;
  ram->size = 0;

  free(ram); // free struct
}

void set_ram(RAM *ram, size_t memory_address, int new_memory_value) {
  if (ram == NULL) {
    puts("memory with error");
    exit(1);
  }

  if (memory_address >= ram->size) {
    printf("Error: address %zu out of bounds (max: %zu)", memory_address, ram->size);
    exit(1);
  }

  ram->memory[memory_address] = new_memory_value;
}

int get_ram(RAM *ram, int memory_address) {
  if (ram == NULL) {
    puts("memory with error");
    exit(1);
  }

  if (memory_address >= ram->size) {
    printf("Error: address %d out of bounds (max: %zu)", memory_address, ram->size);
  }

  return ram->memory[memory_address];
}

/*
 int main(void) {
  RAM *ram = create_empty_ram(10);
  
  printf("endereço: %p\n", ram->memory);
  printf("tamanho: %zu\n", ram->size);
  printf("bytes alocados: %zu\n", ram->size * sizeof(int));
  

  //for (int i = 0; i < ram->size; i++) {
    //printf("%d ", ram->memory[i]);
  //}

  set_ram(ram, 5, 32);
  int* ram_value = get_ram(ram, 5);

  printf("%d \n", *ram_value);

  destroy_ram(ram);

  return 0;
}
*/
