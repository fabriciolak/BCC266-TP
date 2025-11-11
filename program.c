#include <stdio.h>
#include <stdlib.h>

#include "ram.h"
#include "cpu.h"
#include "instruction.h"
#include "opcodes.h"

#define MEMORY_SIZE 100

int main(void)
{
  Register reg = {0, 0, 0, 0, 0};
  // RAM *ram = create_ram(MEMORY_SIZE);
  RAM *ram = create_empty_ram(MEMORY_SIZE);
  Instruction inst[MEMORY_SIZE];

  set_ram(ram, 0, 10);
  set_ram(ram, 1, 20);

  int INDEX_INSTRUCTION = 0;
  inst[0] = (Instruction){ADD, 0, 1, 2};

  while (reg.PC < 3)
  {
    execute_cpu(&reg, ram, inst);
  }

  for (int i = 0; i < 10; i++)
  {
    printf("RAM[%d] = %d\n", i, get_ram(ram, i));
  }

  puts("Instructions: \n");
  // printf("Opcode: %d\nOperando 1: %d\nOperando 2: %d\nOperando 3: %d\n", inst[0].opcode, inst[0].optr1, inst[0].optr2, inst[0].optr2);

  printf("result = %d\n", get_ram(ram, 2));
  destroy_ram(ram);

  return 0;
}
