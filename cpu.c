#include <stdio.h>
#include "instruction.h"

#define MEMORY_SIZE 100
#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4

Instruction memory[MEMORY_SIZE];

/*
  PC: Controla o endereço da instrução atual, controlando o fluxo do programa.
  
  IR: Armazena o opcode da instrução atual, permitindo que a CPU saiba qual operação executar.

  AC: Mantém os resultados das operações e funciona como registrador principal
*/

// segue o principio de: fetch-decode-execute

void execute_cycle(Register *reg) {
  // find the instruction from memory using PC
  reg->IR = memory[reg->PC].opcode;

  // decode and execute
  Instruction current_inst = memory[reg->PC];

  switch (reg->IR) {
    case ADD:
      reg->AC = current_inst.optr1 + current_inst.optr2;
      break;   
    case SUB:
      reg->AC = current_inst.optr1 - current_inst.optr2;
      break;   
    case MUL:
      reg->AC = current_inst.optr1 * current_inst.optr2;
      break;
    case DIV:
      if (current_inst.optr1 != 0) {
        reg->AC = current_inst.optr1 / current_inst.optr2;
      }
      break;
  }

  // increment PC
  reg->PC++;
}

int main(int argc, char *argv[]) {
  Register reg = { 0, 0, 0 };

  memory[0] = (Instruction){ ADD, 5, 5, 0 };
  memory[1] = (Instruction){ MUL, 2, 4, 0 };
  memory[2] = (Instruction){ DIV, 2, 8, 0 };
  memory[3] = (Instruction){ SUB, 2, 6, 0 };
  
  while (reg.PC < 4) {
    execute_cycle(&reg);
    printf("PC=%d, IR=%d, AC=%d\n", reg.PC, reg.IR, reg.AC);
  }

  return 0;
}

