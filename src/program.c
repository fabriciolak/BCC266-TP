#include "include/program.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "include/cpu.h"
#include "include/instruction.h"
#include "include/opcodes.h"
#include "include/ram.h"

#define MEMORY_SIZE 100

/*
sempre reiniciar os registradores
usar uma variavel de controle para quando começar o bloco de instructions.
(principalmente quando usar algum recurso de JUMP)

sempre fazer o reset do estado da CPU antes de executar (AC, IR, PC, R1, R2)
*/

void program_mult(RAM *ram, Register *reg, int multiplicand, int multiplier) {
  Instruction inst[MEMORY_SIZE] = {0};
  int pc = 0;

  // set_ram(ram, 0, 0);  // resultado
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 0, 0};
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 0, 0};

  // set_ram(ram, 1, multiplicand);  // multiplicando
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, multiplicand, 0};
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 1, 0};

  // set_ram(ram, 2, 0);             // contador = 0
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 0, 0};
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 2, 0};

  // set_ram(ram, 3, multiplier);    // multiplier
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, multiplier, 0};
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 3, 0};

  // set_ram(ram, 4, 1);             // constante
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 1, 0};
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 4, 0};

  int loop_start = pc;

  inst[pc++] = (Instruction){ADD, 2, 4, 2};
  inst[pc++] = (Instruction){ADD, 0, 1, 0};
  inst[pc++] = (Instruction){SUB, 3, 2, 5};
  inst[pc++] = (Instruction){JGT, loop_start, 0, 0};
  inst[pc++] = (Instruction){HALT, 0, 0, 0};

  reg->AC = 0;
  reg->IR = 0;
  reg->PC = 0;
  reg->R1 = 0;
  reg->R2 = 0;

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ram, inst);
  }
}

void program_fibonacci(RAM *ram, Register *reg, int term) {
  Instruction inst[MEMORY_SIZE] = {0};
  int pc = 0;

  // INICIALIZAÇÃO DA RAM (instruções 0-7)
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 0, 0};    // R1 = 0
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 0, 0};    // RAM[0] = R1
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 1, 0};    // R1 = 1
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 1, 0};    // RAM[1] = R1
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, term, 0}; // R1 = term
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 2, 0};    // RAM[2] = R1
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 1, 0};    // R1 = 1
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 3, 0};    // RAM[3] = R1

  // LOOP START (instrução 8)
  // guarda o índice da instrução onde o bloco do loop começa
  // para não colocar de modo hardcoded o destino do JUMP
  int loop_start = pc;

  inst[pc++] = (Instruction){ADD, 0, 1, 4}; // proximo = anterior + atual
  inst[pc++] = (Instruction){COPY_RAM_REG, 1, 1, 0}; // R1 = atual
  inst[pc++] = (Instruction){COPY_RAM_REG, 2, 4, 0}; // R2 = proximo
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 0, 0}; // RAM[0] = R1 (anterior)
  inst[pc++] = (Instruction){COPY_REG_RAM, 2, 1, 0}; // RAM[1] = R2 (atual)
  inst[pc++] = (Instruction){SUB, 2, 3, 2};          // contador--

  inst[pc++] = (Instruction){JGT, loop_start, 0,
                             0}; // se contador > 0, volta pro loop_start
  inst[pc++] = (Instruction){HALT, 0, 0, 0};

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ram, inst);
  }
}

void program_sum_matrix(RAM *ram, Register *reg, int size) {
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

void program_div(RAM *ram, Register *reg, int dividend, int divisor) {
  Instruction inst[MEMORY_SIZE] = {0};

  set_ram(ram, 0, dividend);
  set_ram(ram, 1, divisor);
  set_ram(ram, 2, 1);
  set_ram(ram, 3, 0); // result

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

// n (n × (n-1) × (n-2) × ... × 2 × 1).
void program_fat(RAM *ram, Register *reg, int n) {
  Instruction inst[MEMORY_SIZE] = {0};

  set_ram(ram, 0, 1); // RAM[0]: resultado acumulado (começa em 1)
  set_ram(ram, 1, n); // RAM[1]: contador (começa em n)
  set_ram(ram, 2, 1); // RAM[2]: constante 1

  int pc = 0;
  inst[pc++] =
      (Instruction){SUB, 1, 2, 5}; // RAM[5] (ou RAM lixo), AC = RAM[1] - RAM[2]
  inst[pc++] =
      (Instruction){JLT, 5, 0, 5}; // Se AC < 0, salta pra HALT (linha 5)
  inst[pc++] = (Instruction){MUL, 0, 1, 0};  // resultado *= contador
  inst[pc++] = (Instruction){SUB, 1, 2, 1};  // contador -= 1
  inst[pc++] = (Instruction){JUMP, 0, 0, 0}; // volta para o início
  inst[pc++] = (Instruction){HALT, 0, 0, 0}; // fim

  reg->PC = 0;
  reg->IR = 0;
  reg->PC = 0;
  reg->R1 = 0;
  reg->R2 = 0;

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ram, inst);
  }

  printf("Fatorial de %d = %d\n", n, get_ram(ram, 0));
}

int main(void) {
  Register reg = {0, 0, 0, 0, 0};
  RAM *ram = create_empty_ram(MEMORY_SIZE);

  // PROGRAM: 1
  program_mult(ram, &reg, 10, 10);
  printf("Resultado = %d\n", get_ram(ram, 0));

  // PROGRAM: 2
  // program_fibonacci(ram, &reg, 31);
  // printf("Resultado = %d\n", get_ram(ram, 0));

  // PROGRAM: 3
  // program_sum_matrix(ram, &reg, 3);

  // PROGRAM: 4
  // program_div(ram, &reg, 10, 2);
  // printf("Resultado = %d\n", get_ram(ram, 3));

  // PROGRAM: 5
  // program_fat(ram, &reg, 10);
  // printf("Resultado = %d\n", get_ram(ram, 2));

  destroy_ram(ram);
  return 0;
}

/*
  não manipular a RAM diretamente com o set_ram nos programas


*/
