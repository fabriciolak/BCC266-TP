#ifndef CPU_H
#define CPU_H

#include "ram.h"
#include "instruction.h"

void execute_cpu(Register* reg, RAM* ram, Instruction* memory);

#endif
