#ifndef RAM_H
#define RAM_H

#include <stddef.h>
#include "block.h"

typedef struct RAM {
    Block* blocks;  
    size_t num_blocks;
    size_t num_words;
} RAM;

RAM* create_ram(size_t size);
RAM* create_empty_ram(size_t size);
RAM* create_random_ram(size_t size);

int get_ram(RAM* ram, size_t memory_address);
void set_ram(RAM* ram, size_t memory_address, int new_memory_value);
void get_ram_block(RAM* ram, size_t block_address, Block* dest);
void set_ram_block(RAM* ram, size_t block_address, const Block* src);

void destroy_ram(RAM* ram);

static inline size_t word_to_block(size_t word_address) {
  return word_address / WORDS_PER_BLOCK;
}

static inline size_t word_to_offset(size_t word_address) {
  return word_address % WORDS_PER_BLOCK;
}

#endif // RAM_H