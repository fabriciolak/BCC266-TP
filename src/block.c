#include "include/block.h"
#include <string.h>


void block_init(Block* block) {
  if (block == NULL) return;
  for (int i = 0; i < WORDS_PER_BLOCK; i++) {
    block->words[i] = 0;
  }
}

int block_get_word(const Block* block, int word_offset) {
  if (block == NULL || word_offset < 0 || word_offset >= WORDS_PER_BLOCK) {
    return 0;  // Return 0 on error
  }

  return block->words[word_offset];
}

void block_set_word(Block* block, int word_offset, int value) {
  if (block == NULL || word_offset < 0 || word_offset >= WORDS_PER_BLOCK) {
    return;
  }

  block->words[word_offset] = value;
}

void block_copy(Block* dest, const Block* src) {
  if (dest == NULL || src == NULL) return;
  memcpy(dest->words, src->words, WORDS_PER_BLOCK * sizeof(int));
}

/*
  block_init: Zera todos os valores do bloco
  block_get_word: Pega um valor específico (índice 0-3) do bloco
  block_set_word: Define um valor específico no bloco
  block_copy: Copia todos os 4 valores de um bloco para outro (útil quando movemos dados entre RAM e Cache)
*/