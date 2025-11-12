(Instruction){OPERAÇÃO, endereço_A, endereço_B, endereço_destino}

Fica:
RAM[endereço_destino] = RAM[endereço_A] OP RAM[endereço_B]

// RAM[10] = RAM[5] + RAM[6]
(Instruction){ADD, 5, 6, 10}

// RAM[20] = RAM[10] - RAM[15]
(Instruction){SUB, 10, 15, 20}

// RAM[0] = RAM[2] * RAM[3]
(Instruction){MUL, 2, 3, 0}

// RAM[7] = RAM[8] / RAM[9]
(Instruction){DIV, 8, 9, 7}
