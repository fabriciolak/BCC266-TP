#ifndef CPU_H
#define CPU_H

#include "include/ram.h"
#include "include/instruction.h"

void execute_cpu(Register* reg, RAM* ram, Instruction* memory);

#endif
