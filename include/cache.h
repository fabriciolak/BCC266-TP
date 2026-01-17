#ifndef CACHE_H
#define CACHE_H

#include "block.h"

typedef struct CacheLine {
  int valid;              // Is this line valid?  (1 = yes, 0 = no)
  int tag;                // Tag to identify which RAM block is here
  Block data;             // The actual data (4 words)
  int lru_counter;        // For LRU:  timestamp of last access
} CacheLine;

typedef struct Cache {
  CacheLine* lines;       // Array of cache lines
  int num_lines;          // How many lines in this cache
  int access_time;        // Time to access this cache (cycles)
  
  // Statistics
  int hits;               // Number of cache hits
  int misses;             // Number of cache misses
} Cache;

Cache* cache_create(int num_lines, int access_time);
void cache_destroy(Cache* cache);
CacheLine* cache_search(Cache* cache, int block_address, int word_offset);
void cache_load(Cache* cache, int block_address, const Block* block, int current_time);
void cache_write(Cache* cache, int block_address, int word_offset, int value, int current_time);
void cache_reset_stats(Cache* cache);

#endif // CACHE_H

/*

CacheLine:
  valid: Indica se a linha está em uso (1) ou vazia (0)
  tag: Identificador único do bloco da RAM que está armazenado aqui
  data: Os 4 valores (palavras) do bloco
  lru_counter: Timestamp da última vez que foi acessada (para LRU)
  
Cache:
  lines: Array de linhas da cache
  num_lines: Quantas linhas tem (L1=8, L2=16, L3=32)
  access_time: Tempo de acesso em ciclos (L1 rápido, L3 lento)
  hits/misses: Estatísticas para o relatório
*/