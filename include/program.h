#ifndef PROGRAM_H
#define PROGRAM_H

#include "include/instruction.h"
#include "include/ram.h"

void program_mult(RAM* ram, Register* reg, int multiplicand, int multiplier);
void program_div(RAM* ram, Register* reg, int dividend, int divisor);
void program_fat(RAM* ram, Register* reg, int n);
void program_sum_matrix(RAM* ram, Register* reg, int size);
void program_fibonacci(RAM* ram, Register* reg, int term);

#endif  // PROGRAM_H