# TP2 - Fluxo Completo e Diferenças do TP1

## Visão Geral:  TP1 vs TP2

### TP1 - Arquitetura Simples
```
┌─────────────┐
│   Programa  │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│     CPU     │
└──────┬──────┘
       │ get_ram() / set_ram()
       ▼
┌─────────────┐
│     RAM     │  ← 100 ciclos (sempre)
└─────────────┘
```

**Característica:** Acesso direto à RAM, sem otimização. 

---

### TP2 - Arquitetura com Hierarquia de Memória
```
┌─────────────┐
│   Programa  │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│     CPU     │
└──────┬──────┘
       │ ucm_access()
       ▼
┌─────────────┐
│     UCM     │  ← Gerenciador (novo!)
└──────┬──────┘
       │
       ├─→ L1 (8 linhas, 1 ciclo)    ← Novo! 
       │   └─→ HIT?   Retorna rápido
       │
       ├─→ L2 (16 linhas, 10 ciclos) ← Novo!
       │   └─→ HIT?   Carrega L1
       │
       ├─→ L3 (32 linhas, 50 ciclos) ← Novo!
       │   └─→ HIT?  Carrega L2+L1
       │
       └─→ RAM (100 ciclos)
           └─→ MISS total:  Carrega L3+L2+L1
```

**Característica:** Hierarquia de caches, acesso otimizado por localidade.

---

## Fluxo de Execução Detalhado

### TP1 - Fluxo Simples

```
1. Programa define instruções
   └─→ program_mult(ram, reg, 10, 10)

2. Loop de execução:
   └─→ execute_cpu(reg, ram, inst)
       ├─→ Fetch: inst = memory[PC]
       ├─→ Decode: switch(opcode)
       └─→ Execute: 
           ├─→ get_ram(ram, addr)  → 100 ciclos
           ├─→ Computação (CPU)
           └─→ set_ram(ram, addr)  → 100 ciclos

3. Fim (sem métricas)
```

**Tempo por operação ADD:** ~300 ciclos (sempre)

---

### TP2 - Fluxo com Cache

```
1. Programa cria UCM
   └─→ UCM* ucm = ucm_create(ram)
       └─→ Cria L1, L2, L3
       └─→ Inicializa estatísticas

2. Loop de execução:
   └─→ execute_cpu(reg, ucm, inst)
       ├─→ Fetch: inst = memory[PC]
       ├─→ Decode: switch(opcode)
       └─→ Execute:
           ├─→ ucm_access(ucm, addr, READ, 0)
           │   └─→ ucm_read()
           │       ├─→ Busca L1: HIT?  → 1 ciclo ✅
           │       ├─→ Miss L1, busca L2: HIT? → 11 ciclos
           │       ├─→ Miss L2, busca L3: HIT? → 61 ciclos
           │       └─→ Miss L3, busca RAM → 161 ciclos ❌
           │
           ├─→ Computação (CPU)
           │
           └─→ ucm_access(ucm, addr, WRITE, val)
               └─→ ucm_write()
                   ├─→ Atualiza caches (se presente)
                   └─→ SEMPRE escreve RAM (write-through)

3. Imprime estatísticas
   └─→ ucm_print_stats(ucm)
       ├─→ Hit rates de L1, L2, L3
       ├─→ Tempo total e médio
       └─→ Taxa de acesso à RAM

4. Cleanup
   └─→ ucm_destroy(ucm)
```

**Tempo por operação ADD:**
- Primeira vez (miss): ~323 ciclos
- Próximas vezes (hit): ~3-163 ciclos
- **Média muito menor em loops!**

---

## Arquivos Novos (TP2)

### 1. **block.h / block.c**

**O que é:** Unidade básica de transferência (4 palavras).

**Por quê? **
- Caches não carregam palavras individuais, carregam **blocos**
- Simula localidade espacial:  "Se acessou palavra 10, provavelmente acessará 11, 12, 13"

**Analogia:** Em vez de buscar 1 livro na biblioteca, você busca a estante inteira.

**Principais funções:**
```c
block_init()      // Zera bloco
block_get_word()  // Lê 1 palavra do bloco
block_set_word()  // Escreve 1 palavra no bloco
block_copy()      // Copia bloco completo (RAM ↔ Cache)
```

**Impacto:**
- Endereço 10 = Bloco 2 (10/4), Offset 2 (10%4)
- Ao carregar endereço 10, também carrega 8, 9, 11 (bloco completo)

---

### 2. **cache.h / cache.c**

**O que é:** Implementação de uma cache individual (L1, L2 ou L3).

**Por quê?**
- Armazena cópias de blocos da RAM
- Acesso rápido (1-50 ciclos vs 100 ciclos da RAM)

**Estrutura:**
```c
Cache {
  CacheLine* lines;    // Array de linhas
  int num_lines;       // Capacidade (8, 16, 32)
  int access_time;     // Latência (1, 10, 50)
  int hits, misses;    // Estatísticas
}

CacheLine {
  int valid;           // Linha ocupada? 
  int tag;             // Qual bloco da RAM?
  Block data;          // 4 palavras
  int lru_counter;     // Timestamp (LRU)
}
```

**Principais funções:**
```c
cache_create()   // Aloca cache
cache_search()   // Busca bloco (HIT ou MISS)
cache_load()     // Carrega bloco (após MISS)
cache_write()    // Atualiza palavra (se HIT)
```

**Política LRU:**
- Substitui a linha **menos recentemente usada**
- Mantém dados "quentes" na cache

---

### 3. **ucm.h / ucm.c**

**O que é:** **U**nified **C**ache **M**anager - orquestra toda a hierarquia.

**Por quê?**
- Cache individual não sabe dos outros níveis
- UCM coordena L1 ↔ L2 ↔ L3 ↔ RAM
- Implementa políticas globais

**Responsabilidades:**
1. **Leitura (ucm_read):**
   - Busca em cascata (L1 → L2 → L3 → RAM)
   - Propaga dados para cima (inclusive cache)

2. **Escrita (ucm_write):**
   - Atualiza todas as caches onde o bloco está
   - SEMPRE escreve na RAM (write-through)

3. **Estatísticas:**
   - Coleta hits/misses de cada nível
   - Calcula tempos acumulados

**Interface principal:**
```c
ucm_access(ucm, address, UCM_READ, 0);      // Leitura
ucm_access(ucm, address, UCM_WRITE, value); // Escrita
```

---

## Arquivos Modificados

### 1. **RAM (ram.h / ram.c)**

#### Mudança:  Organização por Blocos
```c
// TP1 (implícito):
RAM {
  int* data;  // Array de palavras
}

// TP2 (explícito):
RAM {
  Block* blocks;  // Array de BLOCOS
  size_t num_blocks;
  size_t num_words;
}
```

#### Por quê?
- Caches trabalham com blocos, não palavras
- Facilita transferência RAM ↔ Cache

#### Funções Adicionadas:
```c
get_ram_block()  // Pega bloco completo
set_ram_block()  // Grava bloco completo
word_to_block()  // Converte endereço → bloco
word_to_offset() // Converte endereço → offset no bloco
```

#### Exemplo: 
```c
// TP1: Pegar palavra 10
int val = get_ram(ram, 10);  // Acessa palavra individual

// TP2: Pegar bloco contendo palavra 10
Block blk;
get_ram_block(ram, 2, &blk);  // Bloco 2 = palavras [8,9,10,11]
int val = block_get_word(&blk, 2);  // Offset 2 = palavra 10
```

---

### 2. **CPU (cpu. h / cpu.c)**

#### Mudança: Assinatura da Função
```c
// TP1:
void execute_cpu(Register* reg, RAM* ram, Instruction* memory);

// TP2:
void execute_cpu(Register* reg, UCM* ucm, Instruction* memory);
//                              ↑↑↑↑↑↑↑
```

#### Por quê?
- CPU não deve acessar RAM diretamente
- CPU pede dados ao UCM, UCM decide onde buscar

#### Mudança: Todas as Operações de Memória

**TP1:**
```c
case ADD:
  reg->R1 = get_ram(ram, inst.optr1);      // Direto
  reg->R2 = get_ram(ram, inst.optr2);
  reg->AC = reg->R1 + reg->R2;
  set_ram(ram, inst.optr3, reg->AC);       // Direto
  break;
```

**TP2:**
```c
case ADD:
  reg->R1 = ucm_access(ucm, inst.optr1, UCM_READ, 0);  // Via UCM
  reg->R2 = ucm_access(ucm, inst.optr2, UCM_READ, 0);
  reg->AC = reg->R1 + reg->R2;
  ucm_access(ucm, inst.optr3, UCM_WRITE, reg->AC);     // Via UCM
  break;
```

#### Impacto: 
| Operação | TP1 | TP2 (miss) | TP2 (hit) |
|----------|-----|------------|-----------|
| Leitura | 100 ciclos | 161 ciclos | 1 ciclo ✅ |
| Escrita | 100 ciclos | 161 ciclos | 161 ciclos |

---

### 3. **Program (program.c)**

#### Mudança: Integração com UCM

**TP1:**
```c
void program_mult(RAM* ram, Register* reg, int a, int b) {
  Instruction inst[100];
  // ...  montar instruções ...
  
  while (reg->IR != HALT) {
    execute_cpu(reg, ram, inst);  // Passa RAM
  }
  // Fim (sem estatísticas)
}
```

**TP2:**
```c
void program_mult(RAM* ram, Register* reg, int a, int b) {
  Instruction inst[100];
  // ... montar instruções ...
  
  UCM* ucm = ucm_create(ram);     // ← CRIAR UCM
  
  while (reg->IR != HALT) {
    execute_cpu(reg, ucm, inst);  // ← Passa UCM
  }
  
  printf("\n=== STATISTICS ===\n");
  ucm_print_stats(ucm);           // ← MÉTRICAS
  
  ucm_destroy(ucm);               // ← CLEANUP
}
```

#### Por quê?
- UCM é criado localmente (escopo do programa)
- Cada programa tem suas próprias estatísticas
- Facilita comparação entre programas

#### Novo Programa:  `program_matrix_mult()`

**Por quê?**
- Programas simples (mult, fibonacci) não estressam a cache
- Multiplicação de matrizes tem: 
  - Muitos acessos (~2000 para 10×10)
  - Padrão irregular (3 matrizes diferentes)
  - **Mostra diferenças reais entre configurações**

---

### 4. **Makefile**

#### Mudança: 
```makefile
# TP1:
CFLAGS = -Wall -Wextra ... 

# TP2:
CFLAGS ? = -Wall -Wextra ...
#       ↑
```

#### Por quê?
- Permite testes com diferentes tamanhos de cache: 
```bash
make CFLAGS="-Wall -g -Iinclude -DL1_SIZE=4 -DL2_SIZE=8 -DL3_SIZE=16"
```

- Script bash pode varrer configurações automaticamente

---

## Exemplo Prático: Operação ADD

### Cenário: `RAM[0] = RAM[1] + RAM[2]`

```c
Instruction inst = {ADD, 1, 2, 0};
```

---

### TP1 - Execução

```
1. execute_cpu(reg, ram, inst)
   
2. Decode: opcode = ADD
   
3. Execute:
   ├─→ reg->R1 = get_ram(ram, 1)     [100 ciclos]
   ├─→ reg->R2 = get_ram(ram, 2)     [100 ciclos]
   ├─→ reg->AC = R1 + R2             [  0 ciclos]
   └─→ set_ram(ram, 0, AC)           [100 ciclos]

Total: 300 ciclos
Estatísticas:  Nenhuma
```

---

### TP2 - Primeira Execução (cache vazia)

```
1. execute_cpu(reg, ucm, inst)
   
2. Decode: opcode = ADD
   
3. Execute:
   ├─→ reg->R1 = ucm_access(ucm, 1, READ, 0)
   │   └─→ ucm_read(ucm, 1)
   │       ├─→ Bloco 0 (palavras 0-3)
   │       ├─→ L1: MISS [  1 ciclo]
   │       ├─→ L2: MISS [ 10 ciclos]
   │       ├─→ L3: MISS [ 50 ciclos]
   │       ├─→ RAM: HIT  [100 ciclos]
   │       ├─→ Carrega em L3, L2, L1
   │       └─→ Total: 161 ciclos ❌
   │
   ├─→ reg->R2 = ucm_access(ucm, 2, READ, 0)
   │   └─→ L1: HIT!  (mesmo bloco 0) [1 ciclo] ✅
   │
   ├─→ reg->AC = R1 + R2  [0 ciclos]
   │
   └─→ ucm_access(ucm, 0, WRITE, AC)
       └─→ ucm_write(ucm, 0, AC)
           ├─→ Atualiza L1 (se presente)
           └─→ Escreve RAM [100 ciclos]
           └─→ Total: 161 ciclos (write-through)

Total: 323 ciclos (pior que TP1!)
Estatísticas: 
  - L1: 1 hit, 1 miss
  - Total: 2 hits, 1 miss
```

---

### TP2 - Segunda Execução (dados em cache)

```
1. execute_cpu(reg, ucm, inst)
   
2. Decode: opcode = ADD
   
3. Execute:
   ├─→ reg->R1 = ucm_access(ucm, 1, READ, 0)
   │   └─→ L1: HIT! [1 ciclo] ✅
   │
   ├─→ reg->R2 = ucm_access(ucm, 2, READ, 0)
   │   └─→ L1: HIT! [1 ciclo] ✅
   │
   ├─→ reg->AC = R1 + R2 [0 ciclos]
   │
   └─→ ucm_access(ucm, 0, WRITE, AC)
       └─→ Escreve RAM [161 ciclos] (write-through)

Total: 163 ciclos (46% mais rápido que TP1!) ✅
Estatísticas: 
  - L1: 3 hits, 0 misses
  - Hit rate: 100%
```

---

## Por Que Cada Modificação? 

### Block
**Problema:** Carregar palavra por palavra é ineficiente.  
**Solução:** Transferir blocos (4 palavras) explora localidade espacial.

### Cache
**Problema:** RAM é lenta (100 ciclos).  
**Solução:** Cache armazena cópias rápidas (1-50 ciclos).

### UCM
**Problema:** Múltiplas caches precisam de coordenação.  
**Solução:** UCM gerencia hierarquia e políticas. 

### RAM (modificado)
**Problema:** Interface palavra-a-palavra não suporta blocos.  
**Solução:** Adicionar `get_ram_block()` e `set_ram_block()`.

### CPU (modificado)
**Problema:** Acessava RAM diretamente (sem cache).  
**Solução:** Mudar para `ucm_access()`.

### Program (modificado)
**Problema:** Sem métricas para avaliar performance.  
**Solução:** Criar UCM e imprimir estatísticas.

### Makefile (modificado)
**Problema:** Não consegue testar diferentes configurações.  
**Solução:** `CFLAGS ? =` permite sobrescrita.

---

## Benefícios do TP2

| Aspecto | TP1 | TP2 |
|---------|-----|-----|
| **Realismo** | Simplificado | Próximo de CPUs reais |
| **Performance** | Constante (100 ciclos) | Variável (1-161 ciclos) |
| **Localidade** | Não explorada | Temporal + Espacial |
| **Métricas** | ❌ Nenhuma | ✅ Hit rates, tempos |
| **Análise** | ❌ Impossível | ✅ Comparar configurações |
| **Complexidade** | Baixa | Alta (mas educacional) |

---

## Conclusão

**TP1:** Sistema funcional básico (CPU + RAM).

**TP2:** Sistema realista com: 
- ✅ Hierarquia de memória (L1/L2/L3/RAM)
- ✅ Políticas de cache (LRU, inclusive, write-through)
- ✅ Exploração de localidade
- ✅ Métricas de desempenho
- ✅ Comparação de configurações

**Objetivo Pedagógico:**
Entender **por que** sistemas reais usam caches e **como** elas impactam performance. 