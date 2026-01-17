#ifndef BLOCK_H
#define BLOCK_H

#define WORDS_PER_BLOCK 4  

typedef struct Block {
  int words[WORDS_PER_BLOCK];
} Block;

void block_init(Block* block);
int block_get_word(const Block* block, int word_offset);
void block_set_word(Block* block, int word_offset, int value);
void block_copy(Block* dest, const Block* src);

#endif // BLOCK_H

/*
  WORDS_PER_BLOCK: Define que cada bloco tem 4 palavras (4 inteiros)
  struct Block: Contém apenas um array de 4 inteiros
  Funções auxiliares: Para manipular os blocos de forma segura
*/