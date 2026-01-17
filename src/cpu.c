#include "include/cpu.h"

#include <stdio.h>

#include "include/instruction.h"
#include "include/opcodes.h"
#include "include/ucm.h"

/*
  PC: Controla o endereço da instrução atual, controlando o fluxo do programa.

  IR: Armazena o opcode da instrução atual, permitindo que a CPU saiba qual
  operação executar.

  AC: Mantém os resultados das operações e funciona como registrador principal
*/

// segue o principio de:  fetch-decode-execute

void execute_cpu(Register *reg, UCM *ucm, Instruction *memory) {
  // encontra a instrução da memoria usando PC
  Instruction inst = memory[reg->PC];
  reg->IR = inst.opcode;

  switch (reg->IR) {
  case HALT: 
    puts("program endeed");
    break;

  case ADD:
    // Read operands through UCM
    reg->R1 = ucm_access(ucm, inst.optr1, UCM_READ, 0);
    reg->R2 = ucm_access(ucm, inst.optr2, UCM_READ, 0);

    reg->AC = reg->R1 + reg->R2;

    // Write result through UCM
    ucm_access(ucm, inst.optr3, UCM_WRITE, reg->AC);
    break;

  case SUB:
    // Read operands through UCM
    reg->R1 = ucm_access(ucm, inst.optr1, UCM_READ, 0);
    reg->R2 = ucm_access(ucm, inst.optr2, UCM_READ, 0);

    reg->AC = reg->R1 - reg->R2;

    // Write result through UCM
    ucm_access(ucm, inst.optr3, UCM_WRITE, reg->AC);
    break;

  case MUL:
    // Read operands through UCM
    reg->R1 = ucm_access(ucm, inst.optr1, UCM_READ, 0);
    reg->R2 = ucm_access(ucm, inst.optr2, UCM_READ, 0);

    reg->AC = reg->R1 * reg->R2;

    // Write result through UCM
    ucm_access(ucm, inst.optr3, UCM_WRITE, reg->AC);
    break;

  case DIV:
    // Read operands through UCM
    reg->R1 = ucm_access(ucm, inst.optr1, UCM_READ, 0);
    reg->R2 = ucm_access(ucm, inst.optr2, UCM_READ, 0);

    if (reg->R2 == 0) {
      puts("Error: couldn't divide by zero");
      reg->AC = 0;
      return;
    }

    reg->AC = reg->R1 / reg->R2;

    // Write result through UCM
    ucm_access(ucm, inst.optr3, UCM_WRITE, reg->AC);
    break;

  // carrega um valor do registrador diretamente na ram
  // (optr1 = reg, optr2 = endereço)
  case COPY_REG_RAM:  {
    int which_reg = inst.optr1;
    int address = inst.optr2;

    if (which_reg == 1) {
      ucm_access(ucm, address, UCM_WRITE, reg->R1);
    } else if (which_reg == 2) {
      ucm_access(ucm, address, UCM_WRITE, reg->R2);
    }

    break;
  }

  case COPY_RAM_REG: {
    int which_reg = inst.optr1;
    int address = inst.optr2;

    if (which_reg == 1) {
      reg->R1 = ucm_access(ucm, address, UCM_READ, 0);
    } else if (which_reg == 2) {
      reg->R2 = ucm_access(ucm, address, UCM_READ, 0);
    }

    break;
  }

  // carrega um valor direto no registrador
  // (optr1 = 1 ou 2, optr2 = valor)
  case COPY_EXT_REG: {
    int which_reg = inst. optr1;
    int value = inst.optr2;

    if (which_reg == 1) {
      reg->R1 = value;
    } else if (which_reg == 2) {
      reg->R2 = value;
    }

    break;
  }

  case OBTAIN_REG:  {
    int which_reg = inst.optr1;
    int address = inst.optr2;

    if (which_reg == 1) {
      ucm_access(ucm, address, UCM_WRITE, reg->R1);
    } else if (which_reg == 2) {
      ucm_access(ucm, address, UCM_WRITE, reg->R2);
    }

    break;
  }

  case JUMP: {
    reg->PC = inst.optr1 - 1; // será incrementado no final
    break;
  }

  case JZ: { // Jump if zero
    if (reg->AC == 0) {
      reg->PC = inst.optr1 - 1;
    }
    break;
  }

  case JNZ: { // Jump if not zero
    if (reg->AC != 0) {
      reg->PC = inst.optr1 - 1;
    }
    break;
  }

  case JGT: {
    if (reg->AC > 0) {
      reg->PC = inst.optr1 - 1;
    }
    break;
  }

  case JLT:  {
    if (reg->AC < 0) {
      reg->PC = inst.optr1 - 1;
    }
    break;
  }
  }

  // increment PC
  reg->PC++;
}