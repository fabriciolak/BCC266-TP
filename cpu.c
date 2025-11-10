#include <stdio.h>
#include "instruction.h"
#include "ram.h"
#include "opcodes.h"
#include "cpu.h"

/*
  PC: Controla o endereço da instrução atual, controlando o fluxo do programa.
  
  IR: Armazena o opcode da instrução atual, permitindo que a CPU saiba qual operação executar.

  AC: Mantém os resultados das operações e funciona como registrador principal
*/

// segue o principio de: fetch-decode-execute
//Instruction memory[MEMORY_SIZE];

void execute_cpu(Register *reg, RAM *ram, Instruction *memory) {
  // find the instruction from memory using PC
  Instruction inst = memory[reg->PC];

  switch (inst.opcode) {
    case HALT:
      puts("program endeed");
      return;
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
      int which_reg = inst.optr1; // 1=R1, 2=R2
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
        set_ram(ram , address, reg->R1);
      } else if (which_reg == 2) {
        set_ram(ram, address, reg->R2);
      }

      break;
    }
  }

  // increment PC
  reg->PC++;
}

/*
int main(int argc, char *argv[]) {
  Register reg = { 0, 0, 0 };
  RAM *ram = create_ram(100);

  set_ram(ram, 0, 10); // RAM[0] = 10
  set_ram(ram, 1, 20); // RAM[1] = 20
  set_ram(ram, 3, 2);

  memory[0] = (Instruction){ ADD, 0, 1, 2 };
  memory[1] = (Instruction){ MUL, 3, 2, 4 };

  while (reg.PC < 2) {
    execute_cycle(&reg, ram);
    printf("instruction executed %d\n", reg.PC - 1);
  }

  for (int i = 0; i < 5; i++) {
    printf("RAM[%d] = %d\n", i, get_ram(ram, i));
  }

  destroy_ram(ram);

  return 0;
}
*/
