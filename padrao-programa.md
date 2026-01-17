```c
void program_xxx(RAM *ram, Register *reg, .. .) {
  Instruction inst[MEMORY_SIZE] = {0};
  int pc = 0;

  // ...  instruções ...

  // Reset registers
  reg->AC = 0;
  reg->IR = 0;
  reg->PC = 0;
  reg->R1 = 0;
  reg->R2 = 0;

  // CREATE UCM ✨
  UCM* ucm = ucm_create(ram);

  // Execute with UCM
  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ucm, inst);  // ← PASS UCM! 
  }

  // Print statistics 📊
  printf("\n=== PROGRAM XXX STATISTICS ===\n");
  ucm_print_stats(ucm);

  // Cleanup 🧹
  ucm_destroy(ucm);
}
```