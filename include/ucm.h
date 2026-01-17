#ifndef UCM_H
#define UCM_H

#include "cache.h"
#include "ram.h"

// Operation types
typedef enum {
  UCM_READ,
  UCM_WRITE
} UCM_Operation;

typedef struct UCM {
  Cache* L1;              // Level 1 cache (fastest)
  Cache* L2;              // Level 2 cache
  Cache* L3;              // Level 3 cache (slowest)
  RAM* ram;               // Main memory
  
  int global_time;        // Global timestamp for LRU
  
  // Global statistics
  int total_accesses;     // Total memory accesses
  int total_hits;         // Total cache hits (any level)
  int total_misses;       // Total cache misses (had to go to RAM)
  
  // Time statistics (cycles)
  int total_time;         // Total time spent on memory accesses
} UCM;

UCM* ucm_create(RAM* ram);

void ucm_destroy(UCM* ucm);
int ucm_access(UCM* ucm, int address, UCM_Operation operation, int value);
void ucm_reset_stats(UCM* ucm);
void ucm_print_stats(UCM* ucm);
double ucm_get_hit_rate(UCM* ucm);

#endif // UCM_H