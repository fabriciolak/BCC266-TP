#include "include/ucm.h"

#include <stdio.h>
#include <stdlib.h>

UCM* ucm_create(RAM* ram) {
  if (ram == NULL) return NULL;

  UCM* ucm = (UCM*)malloc(sizeof(UCM));
  if (ucm == NULL) return NULL;

  ucm->L1 = cache_create(2, 1);
  ucm->L2 = cache_create(4, 10);
  ucm->L3 = cache_create(8, 50);

  // Check if all caches were created successfully
  if (ucm->L1 == NULL || ucm->L2 == NULL || ucm->L3 == NULL) {
    if (ucm->L1) cache_destroy(ucm->L1);
    if (ucm->L2) cache_destroy(ucm->L2);
    if (ucm->L3) cache_destroy(ucm->L3);
    free(ucm);
    return NULL;
  }

  ucm->ram = ram;
  ucm->global_time = 0;
  ucm->total_accesses = 0;
  ucm->total_hits = 0;
  ucm->total_misses = 0;
  ucm->total_time = 0;

  return ucm;
}

void ucm_destroy(UCM* ucm) {
  if (ucm == NULL) return;

  if (ucm->L1) cache_destroy(ucm->L1);
  if (ucm->L2) cache_destroy(ucm->L2);
  if (ucm->L3) cache_destroy(ucm->L3);

  free(ucm);
}

static void ucm_handle_miss(UCM* ucm, Cache* cache, int block_address,
                            Block* block) {
  // Load block into this cache
  cache_load(cache, block_address, block, ucm->global_time);
}

static int ucm_read(UCM* ucm, int address) {
  int block_address = word_to_block(address);
  int word_offset = word_to_offset(address);

  ucm->global_time++;
  int access_time = 0;

  // Step 1: Check L1 cache
  CacheLine* line = cache_search(ucm->L1, block_address, word_offset);
  access_time += ucm->L1->access_time;

  if (line != NULL) {
    // L1 HIT!  🎉
    ucm->total_hits++;
    ucm->total_time += access_time;
    line->lru_counter = ucm->global_time;  // Update LRU
    return block_get_word(&line->data, word_offset);
  }

  // Step 2: L1 miss, check L2
  line = cache_search(ucm->L2, block_address, word_offset);
  access_time += ucm->L2->access_time;

  if (line != NULL) {
    // L2 HIT!
    ucm->total_hits++;
    line->lru_counter = ucm->global_time;

    // Load into L1 (inclusive cache)
    ucm_handle_miss(ucm, ucm->L1, block_address, &line->data);

    ucm->total_time += access_time;
    return block_get_word(&line->data, word_offset);
  }

  // Step 3: L2 miss, check L3
  line = cache_search(ucm->L3, block_address, word_offset);
  access_time += ucm->L3->access_time;

  if (line != NULL) {
    // L3 HIT!
    ucm->total_hits++;
    line->lru_counter = ucm->global_time;

    // Load into L2 and L1
    ucm_handle_miss(ucm, ucm->L2, block_address, &line->data);
    ucm_handle_miss(ucm, ucm->L1, block_address, &line->data);

    ucm->total_time += access_time;
    return block_get_word(&line->data, word_offset);
  }

  // Step 4: L3 miss, access RAM (CACHE MISS)
  ucm->total_misses++;

  Block ram_block;
  get_ram_block(ucm->ram, block_address, &ram_block);
  access_time += 100;  // RAM access time

  // Load block into all cache levels (inclusive)
  ucm_handle_miss(ucm, ucm->L3, block_address, &ram_block);
  ucm_handle_miss(ucm, ucm->L2, block_address, &ram_block);
  ucm_handle_miss(ucm, ucm->L1, block_address, &ram_block);

  ucm->total_time += access_time;
  return block_get_word(&ram_block, word_offset);
}

static void ucm_write(UCM* ucm, int address, int value) {
  int block_address = word_to_block(address);
  int word_offset = word_to_offset(address);

  ucm->global_time++;
  int access_time = 0;

  // Write-Through: Update cache if present, then propagate down

  // Try to update L1
  CacheLine* line = cache_search(ucm->L1, block_address, word_offset);
  access_time += ucm->L1->access_time;

  if (line != NULL) {
    // L1 HIT - update value
    ucm->total_hits++;
    block_set_word(&line->data, word_offset, value);
    line->lru_counter = ucm->global_time;
  } else {
    // L1 MISS - load block first
    Block temp_block;
    get_ram_block(ucm->ram, block_address, &temp_block);
    block_set_word(&temp_block, word_offset, value);
    cache_load(ucm->L1, block_address, &temp_block, ucm->global_time);
  }

  // Write-Through: Also update L2
  line = cache_search(ucm->L2, block_address, word_offset);
  access_time += ucm->L2->access_time;

  if (line != NULL) {
    block_set_word(&line->data, word_offset, value);
    line->lru_counter = ucm->global_time;
  }

  // Write-Through: Also update L3
  line = cache_search(ucm->L3, block_address, word_offset);
  access_time += ucm->L3->access_time;

  if (line != NULL) {
    block_set_word(&line->data, word_offset, value);
    line->lru_counter = ucm->global_time;
  }

  // Write-Through:  ALWAYS write to RAM
  set_ram(ucm->ram, address, value);
  access_time += 100;  // RAM access time

  ucm->total_time += access_time;
}

int ucm_access(UCM* ucm, int address, UCM_Operation operation, int value) {
  if (ucm == NULL) return 0;

  ucm->total_accesses++;

  if (operation == UCM_READ) {
    return ucm_read(ucm, address);
  } else {
    ucm_write(ucm, address, value);
    return 0;
  }
}

void ucm_reset_stats(UCM* ucm) {
  if (ucm == NULL) return;

  ucm->global_time = 0;
  ucm->total_accesses = 0;
  ucm->total_hits = 0;
  ucm->total_misses = 0;
  ucm->total_time = 0;

  cache_reset_stats(ucm->L1);
  cache_reset_stats(ucm->L2);
  cache_reset_stats(ucm->L3);
}

double ucm_get_hit_rate(UCM* ucm) {
  if (ucm == NULL || ucm->total_accesses == 0) {
    return 0.0;
  }

  return (double)ucm->total_hits / (double)ucm->total_accesses;
}

void ucm_print_stats(UCM* ucm) {
  if (ucm == NULL) return;

  printf("\n");
  printf("╔════════════════════════════════════════════════╗\n");
  printf("║          MEMORY HIERARCHY STATISTICS          ║\n");
  printf("╠════════════════════════════════════════════════╣\n");

  printf("║ Total Memory Accesses:  %6d                  ║\n",
         ucm->total_accesses);
  printf("║ Total Cache Hits:      %6d                  ║\n", ucm->total_hits);
  printf("║ Total Cache Misses:    %6d                  ║\n",
         ucm->total_misses);
  printf("╠════════════════════════════════════════════════╣\n");

  // L1 statistics
  printf("║ L1 Cache Statistics:                           ║\n");
  printf("║   Hits:   %6d   Misses:  %6d              ║\n", ucm->L1->hits,
         ucm->L1->misses);
  if (ucm->L1->hits + ucm->L1->misses > 0) {
    double l1_hit_rate = (double)ucm->L1->hits /
                         (double)(ucm->L1->hits + ucm->L1->misses) * 100.0;
    printf("║   Hit Rate: %.2f%%                              ║\n",
           l1_hit_rate);
  }
  printf("╠════════════════════════════════════════════════╣\n");

  // L2 statistics
  printf("║ L2 Cache Statistics:                           ║\n");
  printf("║   Hits:   %6d   Misses: %6d              ║\n", ucm->L2->hits,
         ucm->L2->misses);
  if (ucm->L2->hits + ucm->L2->misses > 0) {
    double l2_hit_rate = (double)ucm->L2->hits /
                         (double)(ucm->L2->hits + ucm->L2->misses) * 100.0;
    printf("║   Hit Rate: %.2f%%                              ║\n",
           l2_hit_rate);
  }
  printf("╠════════════════════════════════════════════════╣\n");

  // L3 statistics
  printf("║ L3 Cache Statistics:                           ║\n");
  printf("║   Hits:   %6d   Misses: %6d              ║\n", ucm->L3->hits,
         ucm->L3->misses);
  if (ucm->L3->hits + ucm->L3->misses > 0) {
    double l3_hit_rate = (double)ucm->L3->hits /
                         (double)(ucm->L3->hits + ucm->L3->misses) * 100.0;
    printf("║   Hit Rate: %.2f%%                              ║\n",
           l3_hit_rate);
  }
  printf("╠════════════════════════════════════════════════╣\n");

  // Global statistics
  double overall_hit_rate = ucm_get_hit_rate(ucm) * 100.0;
  printf("║ Overall Hit Rate:  %.2f%%                        ║\n",
         overall_hit_rate);
  printf("║ Total Time (cycles): %6d                    ║\n", ucm->total_time);

  // Average time per access
  if (ucm->total_accesses > 0) {
    double avg_time = (double)ucm->total_time / (double)ucm->total_accesses;
    printf("║ Average Time per Access: %.2f cycles          ║\n", avg_time);
  }

  printf("╚════════════════════════════════════════════════╝\n");
  printf("\n");
}