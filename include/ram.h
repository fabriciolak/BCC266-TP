#ifndef RAM_H
#define RAM_H

#include <stddef.h>

typedef struct RAM {
  int* memory;
  size_t size;
} RAM;

RAM* create_ram(size_t size);
RAM* create_empty_ram(size_t size);
RAM* create_random_ram(size_t size);

int get_ram(RAM* ram, size_t memory_address);
void set_ram(RAM* ram, size_t memory_address, int new_memory_value);

void destroy_ram(RAM* ram);

#endif
