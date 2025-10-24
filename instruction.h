#ifndef INSTRUCTION_H
#define INSTRUCTION_H

typedef struct Instruction {
  int opcode;
  int optr1;
  int optr2;
  int optr3;
} Instruction;

// TODO: Change the definition of Register later

typedef struct Register {
  int PC;  // Program Counter
  int AC;  // Acumulator
  int IR;  // Instruction Register
} Register;

#endif // INSTRUCTION_H
