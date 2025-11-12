#include "cpu.h"

#include <stdio.h>

#include "instruction.h"
#include "opcodes.h"
#include "ram.h"

/*
  PC: Controla o endereço da instrução atual, controlando o fluxo do programa.

  IR: Armazena o opcode da instrução atual, permitindo que a CPU saiba qual
  operação executar.

  AC: Mantém os resultados das operações e funciona como registrador principal
*/

// segue o principio de: fetch-decode-execute
// Instruction memory[MEMORY_SIZE];

void execute_cpu(Register* reg, RAM* ram, Instruction* memory) {
  // find the instruction from memory using PC
  Instruction inst = memory[reg->PC];
  reg->IR = inst.opcode;

  switch (inst.opcode) {
    case HALT:
      puts("program endeed");
      break;
    case ADD:
      reg->R1 = get_ram(ram, inst.optr1);
      reg->R2 = get_ram(ram, inst.optr2);

      reg->AC = reg->R1 + reg->R2;

      set_ram(ram, inst.optr3, reg->AC);

      break;
    case SUB:
      reg->R1 = get_ram(ram, inst.optr1);
      reg->R2 = get_ram(ram, inst.optr2);

      reg->AC = reg->R1 - reg->R2;
      set_ram(ram, inst.optr3, reg->AC);

      break;
    case MUL:
      reg->R1 = get_ram(ram, inst.optr1);
      reg->R2 = get_ram(ram, inst.optr2);

      reg->AC = reg->R1 * reg->R2;
      set_ram(ram, inst.optr3, reg->AC);

      break;
    case DIV:
      reg->R1 = get_ram(ram, inst.optr1);
      reg->R2 = get_ram(ram, inst.optr2);

      if (reg->R2 == 0) {
        puts("Error: couldn't divide by zero");
        reg->AC = 0;
        return;
      }

      reg->AC = reg->R1 / reg->R2;
      set_ram(ram, inst.optr3, reg->AC);

      break;
    case COPY_REG_RAM: {
      int which_reg = inst.optr1;  // 1=R1, 2=R2
      int address = inst.optr2;

      if (which_reg == 1) {
        set_ram(ram, address, reg->R1);
      } else if (which_reg == 2) {
        set_ram(ram, address, reg->R2);
      }

      break;
    }
    case COPY_RAM_REG: {
      int which_reg = inst.optr1;
      int address = inst.optr2;

      if (which_reg == 1) {
        reg->R1 = get_ram(ram, address);
      } else if (which_reg == 2) {
        reg->R2 = get_ram(ram, address);
      }

      break;
    }
    case COPY_EXT_REG: {
      int which_reg = inst.optr1;
      int value = inst.optr2;

      if (which_reg == 1) {
        reg->R1 = value;
      } else if (which_reg == 2) {
        reg->R2 = value;
      }

      break;
    }
    case OBTAIN_REG: {
      int which_reg = inst.optr1;
      int address = inst.optr2;

      if (which_reg == 1) {
        set_ram(ram, address, reg->R1);
      } else if (which_reg == 2) {
        set_ram(ram, address, reg->R2);
      }

      break;
    }
    case JUMP: {
      reg->PC = inst.optr1 - 1;  // sera incrementado no final
      break;
    }
    case JZ: {  // Jump if zero
      if (reg->AC == 0) {
        reg->PC = inst.optr1 - 1;
      }
      break;
    }
    case JNZ: {  // Jump if not zero
      if (reg->AC != 0) {
        reg->PC = inst.optr1 - 1;
      }
      break;
    }
    case JGT: {
      if (reg->AC > 0) {
        reg->PC = inst.optr1 - 1;
      };

      break;
    }
  }

  // increment PC
  reg->PC++;
}
