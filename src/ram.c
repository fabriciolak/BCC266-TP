#include "include/ram.h"
#include <stdlib.h>
#include <string.h>

static size_t calculate_num_blocks(size_t num_words) {
  // Round up:   (num_words + 3) / 4
  return (num_words + WORDS_PER_BLOCK - 1) / WORDS_PER_BLOCK;
}

RAM* create_ram(size_t size) {
  RAM* ram = (RAM*)malloc(sizeof(RAM));
  if (ram == NULL) return NULL;
  
  ram->num_words = size;
  ram->num_blocks = calculate_num_blocks(size);
  
  // Allocate blocks
  ram->blocks = (Block*)malloc(ram->num_blocks * sizeof(Block));
  if (ram->blocks == NULL) {
    free(ram);
    return NULL;
  }
  
  // Initialize all blocks to zero
  for (size_t i = 0; i < ram->num_blocks; i++) {
    block_init(&ram->blocks[i]);
  }
  
  return ram;
}

RAM* create_empty_ram(size_t size) {
  return create_ram(size);
}

RAM* create_random_ram(size_t size) {
  RAM* ram = create_ram(size);
  if (ram == NULL) return NULL;
  
  // Fill with random values
  for (size_t i = 0; i < size; i++) {
    set_ram(ram, i, rand() % 100);  // Random 0-99
  }
  
  return ram;
}

int get_ram(RAM* ram, size_t memory_address) {
  if (ram == NULL || memory_address >= ram->num_words) {
    return 0;  // Error: out of bounds
  }
  
  // Convert word address to block + offset
  size_t block_address = word_to_block(memory_address);
  size_t word_offset = word_to_offset(memory_address);
  
  // Get from the block
  return block_get_word(&ram->blocks[block_address], word_offset);
}

void set_ram(RAM* ram, size_t memory_address, int new_memory_value) {
  if (ram == NULL || memory_address >= ram->num_words) {
    return;  // Error: out of bounds
  }
  
  // Convert word address to block + offset
  size_t block_address = word_to_block(memory_address);
  size_t word_offset = word_to_offset(memory_address);
  
  // Set in the block
  block_set_word(&ram->blocks[block_address], word_offset, new_memory_value);
}

void get_ram_block(RAM* ram, size_t block_address, Block* dest) {
  if (ram == NULL || dest == NULL || block_address >= ram->num_blocks) {
    return;  // Error
  }
  
  // Copy entire block
  block_copy(dest, &ram->blocks[block_address]);
}

void set_ram_block(RAM* ram, size_t block_address, const Block* src) {
  if (ram == NULL || src == NULL || block_address >= ram->num_blocks) {
    return;  // Error
  }
    
  // Copy entire block
  block_copy(&ram->blocks[block_address], src);
}

void destroy_ram(RAM* ram) {
  if (ram == NULL) return;
  
  if (ram->blocks != NULL) {
    free(ram->blocks);
  }
  free(ram);
}