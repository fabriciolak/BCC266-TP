#ifndef CPU_H
#define CPU_H

#include "instruction.h"
#include "ram.h"

void execute_cpu(Register* reg, RAM* ram, Instruction* memory);

#endif
