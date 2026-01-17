#include "include/cache.h"
#include <stdlib.h>
#include <limits.h>

Cache* cache_create(int num_lines, int access_time) {
  Cache* cache = (Cache*)malloc(sizeof(Cache));
  if (cache == NULL) return NULL;
    
  cache->lines = (CacheLine*)malloc(num_lines * sizeof(CacheLine));
  if (cache->lines == NULL) {
    free(cache);
    return NULL;
  }
    
  cache->num_lines = num_lines;
  cache->access_time = access_time;
  cache->hits = 0;
  cache->misses = 0;
  
  // Initialize all lines as invalid (empty)
  for (int i = 0; i < num_lines; i++) {
    cache->lines[i].valid = 0;        // Line is empty
    cache->lines[i].tag = -1;         // No block assigned
    cache->lines[i].lru_counter = 0;  // Never accessed
    block_init(&cache->lines[i].data);
  }
  
  return cache;
}

void cache_destroy(Cache* cache) {
  if (cache == NULL) return;
  
  if (cache->lines != NULL) {
    free(cache->lines);
  }

  free(cache);
}

CacheLine* cache_search(Cache* cache, int block_address, int word_offset) {
  if (cache == NULL) return NULL;
  
  for (int i = 0; i < cache->num_lines; i++) {
    CacheLine* line = &cache->lines[i];
    if (line->valid && line->tag == block_address) {
      cache->hits++;
      return line;
    }
  }
  
  // CACHE MISS 😞
  cache->misses++;
  return NULL;
}

static int cache_find_lru_line(Cache* cache) {
  int lru_index = 0;
  int min_lru = INT_MAX;
  
  // First, try to find an empty line
  for (int i = 0; i < cache->num_lines; i++) {
    if (!cache->lines[i].valid) {
      return i;  // Found empty line, use it! 
    }
  }
  
  // No empty lines, find least recently used (LRU)
  for (int i = 0; i < cache->num_lines; i++) {
    if (cache->lines[i].lru_counter < min_lru) {
      min_lru = cache->lines[i].lru_counter;
      lru_index = i;
    }
  }
  
  return lru_index;
}

void cache_load(Cache* cache, int block_address, const Block* block, int current_time) {
  if (cache == NULL || block == NULL) return;
  
  // Find which line to replace
  int line_index = cache_find_lru_line(cache);
  CacheLine* line = &cache->lines[line_index];
  
  // Load the block into the line
  line->valid = 1;                     // Mark as valid
  line->tag = block_address;           // Set which block this is
  block_copy(&line->data, block);      // Copy the data
  line->lru_counter = current_time;    // Update access time
}

void cache_write(Cache* cache, int block_address, int word_offset, int value, int current_time) {
  if (cache == NULL) return;
  
  // Try to find the block in cache
  CacheLine* line = cache_search(cache, block_address, word_offset);
  
  if (line != NULL) {
    // Block is in cache, update it
    block_set_word(&line->data, word_offset, value);
    line->lru_counter = current_time;  // Update LRU
  }
  // Note: If block not in cache, UCM will handle loading it first
}

void cache_reset_stats(Cache* cache) {
  if (cache == NULL) return;
  
  cache->hits = 0;
  cache->misses = 0;
}