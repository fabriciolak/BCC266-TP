#ifndef RAM_H
#define RAM_H

typedef struct RAM {
  int *memory;
  size_t size;
} RAM;

// create ram
RAM* create_ram(size_t size);
RAM* create_empty_ram(size_t size);
RAM* create_random_ram(size_t size);

// get, set and print_ram
int get_ram(RAM *ram, int memory_address);
void set_ram(RAM *ram, size_t memory_address, int new_memory_value);
//void print_ram(RAM *ram);

void destroy_ram(RAM *ram);

#endif // !RAM_H
