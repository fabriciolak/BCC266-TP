# Como Funciona o Endereçamento?

Cada bloco tem 4 palavras.

Block 0 -> [0, 1, 2, 3]
Block 1 -> [4, 5, 6, 7]
Block 10 -> [40, 41, 42, 43] -> RAM[42] está dentro do bloco 10
Block 11 -> [44, 45, 46, 47]

Para achar o bloco de RAM[42]:

```bash
block number = 42 / 4 = 10
word offset = 42 % 4 = 2
```

Em binário:

42 = 101010₂
Como o bloco tem 4 palavras, usamos 2 bits para o offset.
Os bits mais significativos formam o número do bloco.

1010₂ = 10 → número do bloco
10₂ = 2 → offset da palavra

Block number | Word |
-------------|------|
1010         | 10   |
-------------|------|
10           | 2    |