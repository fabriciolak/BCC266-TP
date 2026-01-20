# Documentação - `block.h` e `block.c`

## **include/block.h**

### Constante
```c
#define WORDS_PER_BLOCK 4
```

**Propósito:** Define o tamanho fixo de um bloco de memória. 

**Valor:** 4 palavras (inteiros)

**Contexto:** 
- Unidade de transferência entre RAM e Cache
- Cada acesso à memória traz/leva um bloco completo
- Simula blocos de cache reais (ex: 16 bytes = 4 palavras de 4 bytes)

---

### Estrutura `Block`
```c
typedef struct Block {
  int words[WORDS_PER_BLOCK];  // Array de 4 inteiros
} Block;
```

**Propósito:** Representa um bloco contíguo de memória.

**Tamanho:** 4 palavras = 16 bytes (em arquiteturas 32-bit)

**Exemplo:**
```c
Block b = {{10, 20, 30, 40}};
// b.words[0] = 10
// b.words[1] = 20
// b.words[2] = 30
// b.words[3] = 40
```

---

## **src/block.c**

### `void block_init(Block* block)`

**Propósito:** Inicializa um bloco zerando todas as palavras.

**Parâmetros:**
- `block`: Ponteiro para o bloco a inicializar

**Funcionamento:**
```
Se block não é NULL:
  Para i de 0 até 3:
    block->words[i] = 0
```

**Uso:**
```c
Block b;
block_init(&b);
// Resultado: b.words = {0, 0, 0, 0}
```

**Contexto:** Usado ao criar linhas de cache vazias.

---

### `int block_get_word(const Block* block, int word_offset)`

**Propósito:** Lê uma palavra específica do bloco.

**Parâmetros:**
- `block`: Ponteiro para o bloco (const = não modifica)
- `word_offset`: Índice da palavra (0-3)

**Retorno:**
- Valor da palavra no offset especificado
- `0` se erro (block NULL ou offset inválido)

**Validação:**
- Verifica se `block != NULL`
- Verifica se `0 ≤ word_offset < 4`

**Uso:**
```c
Block b = {{100, 200, 300, 400}};

int val0 = block_get_word(&b, 0);  // Retorna 100
int val2 = block_get_word(&b, 2);  // Retorna 300
int val5 = block_get_word(&b, 5);  // Retorna 0 (offset inválido)
```

**Contexto:** Usado ao ler dados da cache ou RAM.

---

### `void block_set_word(Block* block, int word_offset, int value)`

**Propósito:** Escreve um valor em uma palavra específica do bloco. 

**Parâmetros:**
- `block`: Ponteiro para o bloco
- `word_offset`: Índice da palavra (0-3)
- `value`: Valor a escrever

**Funcionamento:**
```
Se block não é NULL E offset é válido (0-3):
  block->words[offset] = value
```

**Validação:**
- Verifica se `block != NULL`
- Verifica se `0 ≤ word_offset < 4`

**Uso:**
```c
Block b;
block_init(&b);  // {0, 0, 0, 0}

block_set_word(&b, 0, 50);   // {50, 0, 0, 0}
block_set_word(&b, 2, 75);   // {50, 0, 75, 0}
block_set_word(&b, 3, 100);  // {50, 0, 75, 100}
```

**Contexto:** Usado ao escrever dados na cache ou RAM. 

---

### `void block_copy(Block* dest, const Block* src)`

**Propósito:** Copia todos os dados de um bloco para outro. 

**Parâmetros:**
- `dest`: Bloco de destino
- `src`: Bloco de origem (const = não modifica)

**Funcionamento:**
```
Se dest e src não são NULL:
  Usa memcpy para copiar 4 inteiros de uma vez
  (16 bytes em arquiteturas 32-bit)
```

**Implementação:**
```c
memcpy(dest->words, src->words, WORDS_PER_BLOCK * sizeof(int));
```

**Vantagem:** Mais eficiente que loop manual palavra por palavra.

**Uso:**
```c
Block src = {{10, 20, 30, 40}};
Block dest;

block_copy(&dest, &src);
// dest.words = {10, 20, 30, 40}

// src permanece inalterado (const)
```

**Contexto:** Usado ao carregar blocos da RAM para cache (cache miss).

---

## Fluxo de Uso Completo

### 1. Inicialização
```c
Block b;
block_init(&b);
// b = {0, 0, 0, 0}
```

### 2. Escrita Individual
```c
block_set_word(&b, 0, 100);
block_set_word(&b, 1, 200);
block_set_word(&b, 2, 300);
block_set_word(&b, 3, 400);
// b = {100, 200, 300, 400}
```

### 3. Leitura Individual
```c
int primeiro = block_get_word(&b, 0);  // 100
int terceiro = block_get_word(&b, 2);  // 300
```

### 4. Cópia Completa
```c
Block backup;
block_copy(&backup, &b);
// backup = {100, 200, 300, 400}
```

---

## Mapeamento de Endereços

### Conversão:  Endereço → (Bloco, Offset)

**Fórmula:**
```c
int block_address = address / WORDS_PER_BLOCK;  // Número do bloco
int word_offset = address % WORDS_PER_BLOCK;    // Posição dentro do bloco
```

**Exemplo:**
```
Endereço  | Bloco | Offset | Descrição
----------|-------|--------|----------------------------------
    0     |   0   |   0    | Primeira palavra do bloco 0
    1     |   0   |   1    | Segunda palavra do bloco 0
    2     |   0   |   2    | Terceira palavra do bloco 0
    3     |   0   |   3    | Quarta palavra do bloco 0
    4     |   1   |   0    | Primeira palavra do bloco 1
    5     |   1   |   1    | Segunda palavra do bloco 1
    6     |   1   |   2    | Terceira palavra do bloco 1
    7     |   1   |   3    | Quarta palavra do bloco 1
    8     |   2   |   0    | Primeira palavra do bloco 2
   ...    |  ...  |  ...   | ...
```

**Funções Auxiliares (Exemplo):**
```c
// Converte endereço de palavra para endereço de bloco
static inline int word_to_block(int address) {
  return address / WORDS_PER_BLOCK;
}

// Extrai offset dentro do bloco
static inline int word_to_offset(int address) {
  return address % WORDS_PER_BLOCK;
}
```

---

## Exemplo Prático: Cache Miss

### Cenário: CPU quer ler endereço 10

```c
// 1. Calcular bloco e offset
int address = 10;
int block_num = address / 4;  // = 2 (bloco 2)
int offset = address % 4;      // = 2 (terceira palavra)

// 2. Verificar cache (MISS!)
CacheLine* line = cache_search(l1, block_num, offset);
if (line == NULL) {
  // MISS!  Precisa buscar na RAM
  
  // 3. Buscar bloco completo da RAM
  Block ram_block;
  get_ram_block(ram, block_num, &ram_block);
  // ram_block = {RAM[8], RAM[9], RAM[10], RAM[11]}
  
  // 4. Carregar bloco na cache
  cache_load(l1, block_num, &ram_block, current_time);
  
  // 5. Agora pode ler a palavra desejada
  line = cache_search(l1, block_num, offset);
  int value = block_get_word(&line->data, offset);  // RAM[10]
}
```

---

## Exemplo Prático:  Escrita na RAM

### Cenário:  Escrever valor 999 no endereço 5

```c
// 1. Calcular bloco e offset
int address = 5;
int block_num = 1;   // 5 / 4 = 1
int offset = 1;      // 5 % 4 = 1

// 2. Buscar bloco na RAM
Block block;
get_ram_block(ram, block_num, &block);
// block = {RAM[4], RAM[5], RAM[6], RAM[7]}

// 3. Modificar palavra específica
block_set_word(&block, offset, 999);
// block = {RAM[4], 999, RAM[6], RAM[7]}

// 4. Escrever bloco de volta na RAM
set_ram_block(ram, block_num, &block);
```

---

## Vantagens da Estrutura Block

### 1. Localidade Espacial
```
Ao carregar endereço 8, também carrega 9, 10, 11
Se programa acessar 9 logo depois → HIT!  (já está na cache)
```

### 2. Eficiência de Transferência
```
1 transferência de bloco (4 palavras) é mais eficiente que
4 transferências individuais
```

### 3. Simplicidade
```
Cache gerencia blocos, não palavras individuais
Menor overhead de controle
```

---

## Comparação:  Com e Sem Blocos

### Sem Blocos (Palavras Individuais)
```c
// Cache de 8 palavras
int cache[8];
int tags[8];

// Ler endereço 10: transfere 1 palavra
cache[linha] = RAM[10];

// Ler endereço 11: transfere 1 palavra (outra vez!)
cache[linha] = RAM[11];
```
**Desvantagem:** 2 transferências para dados próximos. 

---

### Com Blocos (Sistema Atual)
```c
// Cache de 8 linhas, cada uma com 4 palavras
Block cache[8];

// Ler endereço 10: transfere bloco inteiro (10,11,12,13)
block_copy(&cache[linha], &RAM_block[2]);

// Ler endereço 11: JÁ ESTÁ NA CACHE! (HIT)
int value = block_get_word(&cache[linha], offset);
```
**Vantagem:** 1 transferência, múltiplos acessos.

---

## Relação com Outros Módulos

### Block ← RAM
```c
// RAM contém blocos
typedef struct RAM {
  Block* blocks;  // Array de blocos
  int num_blocks;
} RAM;

// Função para buscar bloco da RAM
void get_ram_block(RAM* ram, int block_num, Block* dest) {
  block_copy(dest, &ram->blocks[block_num]);
}
```

### Block ← Cache
```c
// CacheLine contém um bloco
typedef struct CacheLine {
  int valid;
  int tag;
  Block data;  // ← Bloco de 4 palavras
  int lru_counter;
} CacheLine;
```

### Block ← UCM
```c
// UCM usa blocos para transferências
void ucm_handle_miss(UCM* ucm, Cache* cache, int block_addr, Block* block) {
  cache_load(cache, block_addr, block, ucm->global_time);
}
```

---

## Casos de Teste

### Teste 1: Inicialização
```c
Block b;
block_init(&b);
assert(block_get_word(&b, 0) == 0);
assert(block_get_word(&b, 1) == 0);
assert(block_get_word(&b, 2) == 0);
assert(block_get_word(&b, 3) == 0);
```

### Teste 2: Escrita e Leitura
```c
Block b;
block_init(&b);
block_set_word(&b, 2, 42);
assert(block_get_word(&b, 2) == 42);
assert(block_get_word(&b, 0) == 0);  // Outras palavras intactas
```

### Teste 3: Cópia
```c
Block src = {{1, 2, 3, 4}};
Block dest;
block_copy(&dest, &src);
assert(block_get_word(&dest, 0) == 1);
assert(block_get_word(&dest, 3) == 4);
```

### Teste 4: Validação de Offset
```c
Block b;
block_init(&b);
int val = block_get_word(&b, 10);  // Offset inválido
assert(val == 0);  // Retorna 0 (erro)

block_set_word(&b, -1, 100);  // Offset inválido
// Não faz nada (sem crash)
```

---

## Resumo de Funções

| Função | Propósito | Modifica?  | Retorno |
|--------|-----------|-----------|---------|
| `block_init()` | Zera bloco | Sim | void |
| `block_get_word()` | Lê palavra | Não | int (valor) |
| `block_set_word()` | Escreve palavra | Sim | void |
| `block_copy()` | Copia bloco completo | Sim (dest) | void |

---

## Performance

### Custo de `block_copy()`
```c
memcpy(dest, src, 16 bytes)  // Otimizado pelo compilador
```
**Tempo:** O(1) - cópia de tamanho fixo  
**Espaço:** O(1) - sem alocação dinâmica

### Custo de `block_get_word()`
```c
return block->words[offset];  // Acesso direto ao array
```
**Tempo:** O(1) - acesso indexado  
**Espaço:** O(1) - sem alocação

### Custo de `block_set_word()`
```c
block->words[offset] = value;  // Atribuição direta
```
**Tempo:** O(1)  
**Espaço:** O(1)

**Conclusão:** Todas as operações são O(1) - extremamente eficientes. 