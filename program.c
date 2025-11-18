#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cpu.h"
#include "instruction.h"
#include "opcodes.h"
#include "ram.h"

#define MEMORY_SIZE 100

void program_mult(RAM* ram, Register* reg, int multiplicand, int multiplier) {
  Instruction inst[MEMORY_SIZE] = {0};

  set_ram(ram, 0, 0);             // resultado
  set_ram(ram, 1, multiplicand);  // multiplicando
  set_ram(ram, 2, 0);             // contador = 0
  set_ram(ram, 3, multiplier);    // multiplier
  set_ram(ram, 4, 1);             // constante

  inst[0] = (Instruction){ADD, 2, 4, 2};
  inst[1] = (Instruction){ADD, 0, 1, 0};
  inst[2] = (Instruction){SUB, 3, 2, 5};
  inst[3] = (Instruction){JGT, 0, 0, 0};
  inst[4] = (Instruction){HALT, 0, 0, 0};

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ram, inst);
  }
}

void program_fibonacci(RAM* ram, Register* reg, int term) {
  Instruction inst[MEMORY_SIZE] = {0};

  set_ram(ram, 0, 0);     // anterior = 0
  set_ram(ram, 1, 1);     // atual = 1
  set_ram(ram, 2, term);  // contador = n
  set_ram(ram, 3, 1);     // constante

  // LOOP START
  inst[0] = (Instruction){ADD, 0, 1, 4};           // proximo = anterior + atual
  inst[1] = (Instruction){COPY_RAM_REG, 1, 1, 0};  // temp = atual
  inst[2] = (Instruction){COPY_RAM_REG, 2, 4, 0};  // current = proximo
  inst[3] = (Instruction){COPY_REG_RAM, 1, 0, 0};  // anterior = temp
  inst[4] = (Instruction){COPY_REG_RAM, 2, 1, 0};  // anterior = temp
  inst[5] = (Instruction){SUB, 2, 3, 2};           // --contator

  inst[6] = (Instruction){JGT, 0, 0, 0};
  // se o contador maior que 0, volta pro loop
  inst[7] = (Instruction){HALT, 0, 0, 0};

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ram, inst);
  }
}

void program_sum_matrix(RAM* ram, Register* reg, int size) {
  Instruction inst[MEMORY_SIZE] = {0};

  int n_elements = size * size;
  int delta = n_elements;

  srand(time(NULL));

  for (int i = 0; i < n_elements; i++) {
    set_ram(ram, i, rand() % 100);
    set_ram(ram, delta + i, rand() % 100);
  }

  int pc = 0;

  for (int i = 0; i < n_elements; i++) {
    inst[pc++] = (Instruction){ADD, i, delta + i, 2 * delta + i};
  }

  inst[pc++] = (Instruction){HALT, 0, 0, 0};

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ram, inst);
  }

  printf("Matriz A\n");
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      printf("\t%d ", get_ram(ram, i * size + j));
    }
    printf("\n");
  }
  printf("\n");
  printf("Matriz B\n");
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      printf("\t%d ", get_ram(ram, delta + i * size + j));
    }
    printf("\n");
  }
  printf("\n");
}

void program_div(RAM* ram, Register* reg, int dividend, int divisor) {
  Instruction inst[MEMORY_SIZE] = {0};

  set_ram(ram, 0, dividend);
  set_ram(ram, 1, divisor);
  set_ram(ram, 2, 1);
  set_ram(ram, 3, 0);  // result

  int pc = 0;

  inst[pc++] = (Instruction){ADD, 3, 2, 3};
  inst[pc++] = (Instruction){SUB, 0, 1, 0};
  inst[pc++] = (Instruction){JGT, 0, 0, 0};
  inst[pc++] = (Instruction){HALT, 0, 0, 0};

  // reg->PC = 0;

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ram, inst);
  }
}

int main(void) {
  Register reg = {0, 0, 0, 0, 0};
  RAM* ram = create_empty_ram(MEMORY_SIZE);

  // program_fibonacci(ram, &reg, 10);
  program_sum_matrix(ram, &reg, 3);
  program_div(ram, &reg, 10, 2);
  // printf("Resultado = %d\n", get_ram(ram, 3));

  destroy_ram(ram);
  return 0;
}