#ifndef CPU_H
#define CPU_H

#include "ram.h"

// CPU
typedef struct CPU {
  Registers regs;
  RAM *ram;       // pointer to RAM
  int state;      // running, stoped, etc
} CPU;

#endif
