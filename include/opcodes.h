#ifndef OPCODES_H
#define OPCODES_H

#define MEMORY_SIZE 100
#define HALT -1

#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4

#define COPY_REG_RAM 5  // copy value to another address
#define COPY_RAM_REG 6
#define COPY_EXT_REG 7
#define OBTAIN_REG 8

#define JUMP 10  // Salto incondicional
#define JZ 11    // (AC == 0)
#define JNZ 12   // (AC != 0)
#define JGT 13   // (AC > 0)
#define JLT 14   // (AC < 0)

#endif  // !OPCODES_H
