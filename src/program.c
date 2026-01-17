#include "include/program.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "include/cpu.h"
#include "include/instruction.h"
#include "include/opcodes.h"
#include "include/ram.h"
#include "include/ucm.h"

#define MEMORY_SIZE 100

/*
sempre reiniciar os registradores
usar uma variavel de controle para quando começar o bloco de instructions.
(principalmente quando usar algum recurso de JUMP)

sempre fazer o reset do estado da CPU antes de executar (AC, IR, PC, R1, R2)
*/

void program_mult(RAM* ram, Register* reg, int multiplicand, int multiplier) {
  Instruction inst[MEMORY_SIZE] = {0};
  int pc = 0;

  // RAM[0] = 0 (resultado)
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 0, 0};
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 0, 0};

  // RAM[1] = multiplicand
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, multiplicand, 0};
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 1, 0};

  // RAM[2] = 0 (contador)
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 0, 0};
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 2, 0};

  // RAM[3] = multiplier
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, multiplier, 0};
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 3, 0};

  // RAM[4] = 1 (constante)
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

  // CREATE UCM
  UCM* ucm = ucm_create(ram);

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ucm, inst);  // ← PASS UCM!
  }

  // Print statistics
  printf("\n=== PROGRAM MULT STATISTICS ===\n");
  ucm_print_stats(ucm);

  // Cleanup
  ucm_destroy(ucm);
}

void program_fibonacci(RAM* ram, Register* reg, int term) {
  Instruction inst[MEMORY_SIZE] = {0};
  int pc = 0;

  // INICIALIZAÇÃO DA RAM (instruções 0-7)
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 0, 0};     // R1 = 0
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 0, 0};     // RAM[0] = R1
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 1, 0};     // R1 = 1
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 1, 0};     // RAM[1] = R1
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, term, 0};  // R1 = term
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 2, 0};     // RAM[2] = R1
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 1, 0};     // R1 = 1
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 3, 0};     // RAM[3] = R1

  // LOOP START (instrução 8)
  int loop_start = pc;

  inst[pc++] = (Instruction){ADD, 0, 1, 4};  // proximo = anterior + atual
  inst[pc++] = (Instruction){COPY_RAM_REG, 1, 1, 0};  // R1 = atual
  inst[pc++] = (Instruction){COPY_RAM_REG, 2, 4, 0};  // R2 = proximo
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 0, 0};  // RAM[0] = R1 (anterior)
  inst[pc++] = (Instruction){COPY_REG_RAM, 2, 1, 0};  // RAM[1] = R2 (atual)
  inst[pc++] = (Instruction){SUB, 2, 3, 2};           // contador--

  inst[pc++] = (Instruction){JGT, loop_start, 0, 0};  // se contador > 0, volta
  inst[pc++] = (Instruction){HALT, 0, 0, 0};

  reg->AC = 0;
  reg->IR = 0;
  reg->PC = 0;
  reg->R1 = 0;
  reg->R2 = 0;

  // CREATE UCM
  UCM* ucm = ucm_create(ram);

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ucm, inst);  // ← PASS UCM!
  }

  // Print statistics
  printf("\n=== PROGRAM FIBONACCI STATISTICS ===\n");
  ucm_print_stats(ucm);

  // Cleanup
  ucm_destroy(ucm);
}

void program_sum_matrix(RAM* ram, Register* reg, int size) {
  Instruction inst[MEMORY_SIZE] = {0};

  int n_elements = size * size;
  int delta = n_elements;

  srand(time(NULL));

  // Pre-load matrices (acceptable for test setup)
  // In real scenario, this would also be done via instructions
  for (int i = 0; i < n_elements; i++) {
    set_ram(ram, i, rand() % 100);
    set_ram(ram, delta + i, rand() % 100);
  }

  int pc = 0;

  // Generate addition instructions
  for (int i = 0; i < n_elements; i++) {
    inst[pc++] = (Instruction){ADD, i, delta + i, 2 * delta + i};
  }

  inst[pc++] = (Instruction){HALT, 0, 0, 0};

  reg->AC = 0;
  reg->IR = 0;
  reg->PC = 0;
  reg->R1 = 0;
  reg->R2 = 0;

  // CREATE UCM
  UCM* ucm = ucm_create(ram);

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ucm, inst);  // ← PASS UCM!
  }

  // Print matrices
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

  // Print statistics
  printf("\n=== PROGRAM SUM MATRIX STATISTICS ===\n");
  ucm_print_stats(ucm);

  // Cleanup
  ucm_destroy(ucm);
}

void program_div(RAM* ram, Register* reg, int dividend, int divisor) {
  Instruction inst[MEMORY_SIZE] = {0};
  int pc = 0;

  // Initialize RAM via instructions (CORRECT WAY)

  // RAM[0] = dividend
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, dividend, 0};  // R1 = dividend
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 0, 0};         // RAM[0] = R1

  // RAM[1] = divisor
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, divisor, 0};  // R1 = divisor
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 1, 0};        // RAM[1] = R1

  // RAM[2] = 1
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 1, 0};  // R1 = 1
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 2, 0};  // RAM[2] = R1

  // RAM[3] = 0 (result)
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 0, 0};  // R1 = 0
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 3, 0};  // RAM[3] = R1

  // Division algorithm starts at instruction 8
  int loop_start = pc;

  inst[pc++] = (Instruction){ADD, 3, 2, 3};           // result++
  inst[pc++] = (Instruction){SUB, 0, 1, 0};           // dividend -= divisor
  inst[pc++] = (Instruction){JGT, loop_start, 0, 0};  // if dividend > 0, loop
  inst[pc++] = (Instruction){HALT, 0, 0, 0};

  reg->AC = 0;
  reg->IR = 0;
  reg->PC = 0;
  reg->R1 = 0;
  reg->R2 = 0;

  // CREATE UCM
  UCM* ucm = ucm_create(ram);

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ucm, inst);  // ← PASS UCM!
  }

  // Print statistics
  printf("\n=== PROGRAM DIV STATISTICS ===\n");
  ucm_print_stats(ucm);

  // Cleanup
  ucm_destroy(ucm);
}

// n (n × (n-1) × (n-2) × ... × 2 × 1).
void program_fat(RAM* ram, Register* reg, int n) {
  Instruction inst[MEMORY_SIZE] = {0};
  int pc = 0;

  // Initialize RAM via instructions (CORRECT WAY)

  // RAM[0] = 1 (resultado acumulado)
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 1, 0};  // R1 = 1
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 0, 0};  // RAM[0] = R1

  // RAM[1] = n (contador)
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, n, 0};  // R1 = n
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 1, 0};  // RAM[1] = R1

  // RAM[2] = 1 (constante)
  inst[pc++] = (Instruction){COPY_EXT_REG, 1, 1, 0};  // R1 = 1
  inst[pc++] = (Instruction){COPY_REG_RAM, 1, 2, 0};  // RAM[2] = R1

  // Factorial algorithm starts at instruction 6
  int loop_start = pc;

  inst[pc++] = (Instruction){SUB, 1, 2, 5};       // AC = RAM[1] - RAM[2]
  inst[pc++] = (Instruction){JLT, pc + 4, 0, 0};  // If AC < 0, jump to HALT
  inst[pc++] = (Instruction){MUL, 0, 1, 0};       // result *= counter
  inst[pc++] = (Instruction){SUB, 1, 2, 1};       // counter -= 1
  inst[pc++] = (Instruction){JUMP, loop_start, 0, 0};  // Jump back to loop
  inst[pc++] = (Instruction){HALT, 0, 0, 0};           // End

  reg->AC = 0;
  reg->IR = 0;
  reg->PC = 0;
  reg->R1 = 0;
  reg->R2 = 0;

  // CREATE UCM
  UCM* ucm = ucm_create(ram);

  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ucm, inst);  // ← PASS UCM!
  }

  printf("Fatorial de %d = %d\n", n, get_ram(ram, 0));

  // Print statistics
  printf("\n=== PROGRAM FAT STATISTICS ===\n");
  ucm_print_stats(ucm);

  // Cleanup
  ucm_destroy(ucm);
}

// Adicione no src/program.c

// Matrix multiplication:  C = A × B (size × size matrices)
// Acessa MUITA memória, causa muitos cache misses!
void program_matrix_mult(RAM* ram, Register* reg, int size) {
  printf("\n=== PROGRAM MATRIX MULTIPLICATION (%dx%d) ===\n\n", size, size);

  // Memory layout:
  // Matrix A:  RAM[0] to RAM[size*size-1]
  // Matrix B: RAM[size*size] to RAM[2*size*size-1]
  // Matrix C (result): RAM[2*size*size] to RAM[3*size*size-1]

  int base_a = 0;
  int base_b = size * size;
  int base_c = 2 * size * size;

  // Initialize matrices A and B with small values
  for (int i = 0; i < size * size; i++) {
    set_ram(ram, base_a + i, (i % 10) + 1);  // A[i] = 1.. 10
    set_ram(ram, base_b + i, (i % 10) + 1);  // B[i] = 1..10
    set_ram(ram, base_c + i, 0);             // C[i] = 0
  }

  printf("Matrices initialized.  Starting multiplication...\n");

  UCM* ucm = ucm_create(ram);
  if (ucm == NULL) {
    printf("Error:  Could not create UCM\n");
    return;
  }

  ucm_reset_stats(ucm);

  // Matrix multiplication:  C[i][j] = sum(A[i][k] * B[k][j])
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      int sum = 0;

      for (int k = 0; k < size; k++) {
        // Access A[i][k]
        int a_addr = base_a + (i * size + k);
        int a_val = ucm_access(ucm, a_addr, UCM_READ, 0);

        // Access B[k][j]
        int b_addr = base_b + (k * size + j);
        int b_val = ucm_access(ucm, b_addr, UCM_READ, 0);

        sum += a_val * b_val;
      }

      // Write C[i][j]
      int c_addr = base_c + (i * size + j);
      ucm_access(ucm, c_addr, UCM_WRITE, sum);
    }
  }

  printf("program endeed\n");
  ucm_print_stats(ucm);

  // Show sample result
  int c00 = get_ram(ram, base_c);
  printf("C[0][0] = %d\n", c00);

  ucm_destroy(ucm);
}

int main(void) {
  Register reg = {0, 0, 0, 0, 0};
  RAM* ram = create_empty_ram(MEMORY_SIZE);

  // PROGRAM:  1
  // program_mult(ram, &reg, 10, 10);
  // printf("Resultado = %d\n", get_ram(ram, 0));

  // PROGRAM: 2
  // program_fibonacci(ram, &reg, 10);
  // printf("Resultado = %d\n", get_ram(ram, 0));

  // PROGRAM: 3
  // program_sum_matrix(ram, &reg, 100);

  // PROGRAM MULT MATRIX
  program_matrix_mult(ram, &reg, 10);  // 10x10 matrix

  // PROGRAM: 4
  // program_div(ram, &reg, 10, 2);
  // printf("Resultado = %d\n", get_ram(ram, 3));

  // PROGRAM: 5
  // program_fat(ram, &reg, 10);

  destroy_ram(ram);
  return 0;
}