#ifndef CPU_H
#define CPU_H

#include "ram.h"
#include "instruction.h"
#include "ucm.h"  // ← ADD THIS

void execute_cpu(Register* reg, UCM* ucm, Instruction* memory);  // ← CHANGED

#endif