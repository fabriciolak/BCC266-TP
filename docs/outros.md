# Documentação - Modificações em Arquivos Existentes

## 1. **Makefile**

### Modificação Principal
```makefile
# ANTES (TP1):
CFLAGS = -Wall -Wextra -std=c11 -g -Iinclude -I.  

# DEPOIS (TP2):
CFLAGS ? = -Wall -Wextra -std=c11 -g -Iinclude -I.  
#       ↑
#   Operador ? =
```

**Propósito:** Permitir sobrescrita de `CFLAGS` via linha de comando. 

**Diferença:**
- `=` → **Atribuição definitiva** (não pode ser sobrescrito)
- `?=` → **Atribuição condicional** (só define se ainda não foi definido)

**Uso:**
```bash
# ANTES (não funciona):
make CFLAGS="-DL1_SIZE=4"
# Resultado: usa CFLAGS original (ignora -DL1_SIZE=4)

# DEPOIS (funciona):
make CFLAGS="-Wall -Wextra -std=c11 -g -Iinclude -I.   -DL1_SIZE=4"
# Resultado: usa CFLAGS fornecido (inclui -DL1_SIZE=4)
```

**Contexto:** Necessário para testar diferentes tamanhos de cache dinamicamente.

---

## 2. **src/cpu.c** e **include/cpu.h**

### Modificação na Assinatura
```c
// ANTES (TP1):
void execute_cpu(Register* reg, RAM* ram, Instruction* memory);

// DEPOIS (TP2):
void execute_cpu(Register* reg, UCM* ucm, Instruction* memory);
//                              ↑↑��↑↑↑↑
//                           UCM ao invés de RAM
```

**Propósito:** CPU agora acessa memória através do UCM (com hierarquia de caches).

---

### Modificação no Header (`cpu.h`)
```c
// ANTES: 
#include "ram.h"

// DEPOIS:
#include "ram.h"
#include "ucm.h"  // ← ADICIONADO
```

---

### Modificações nas Operações Aritméticas

#### **ANTES (TP1) - Acesso Direto à RAM:**
```c
case ADD: 
  reg->R1 = get_ram(ram, inst.optr1);  // ← Direto da RAM
  reg->R2 = get_ram(ram, inst.optr2);
  reg->AC = reg->R1 + reg->R2;
  set_ram(ram, inst.optr3, reg->AC);   // ← Direto na RAM
  break;
```

**Característica:** Sem cache, sempre acessa RAM (100 ciclos).

---

#### **DEPOIS (TP2) - Acesso Através do UCM:**
```c
case ADD:
  reg->R1 = ucm_access(ucm, inst.optr1, UCM_READ, 0);  // ← Através UCM
  reg->R2 = ucm_access(ucm, inst.optr2, UCM_READ, 0);
  reg->AC = reg->R1 + reg->R2;
  ucm_access(ucm, inst.optr3, UCM_WRITE, reg->AC);     // ← Através UCM
  break;
```

**Característica:** Usa hierarquia de caches (L1→L2→L3→RAM).

---

### Lista Completa de Operações Modificadas

Todas as seguintes operações trocaram `get_ram/set_ram` por `ucm_access`:

| Opcode | Leituras | Escritas |
|--------|----------|----------|
| `ADD` | 2 (optr1, optr2) | 1 (optr3) |
| `SUB` | 2 (optr1, optr2) | 1 (optr3) |
| `MUL` | 2 (optr1, optr2) | 1 (optr3) |
| `DIV` | 2 (optr1, optr2) | 1 (optr3) |
| `COPY_REG_RAM` | 0 | 1 (address) |
| `COPY_RAM_REG` | 1 (address) | 0 |
| `OBTAIN_REG` | 0 | 1 (address) |

**Operações NÃO modificadas:**
- `HALT`, `COPY_EXT_REG`, `JUMP`, `JZ`, `JNZ`, `JGT`, `JLT` (não acessam memória)

---

### Exemplo Comparativo

#### Programa:  `a = b + c`
```c
// Suponha:  b está em RAM[10], c está em RAM[11], resultado em RAM[12]
Instruction inst = {ADD, 10, 11, 12};
```

**TP1 (Sem Cache):**
```
1. get_ram(ram, 10)     → 100 ciclos
2. get_ram(ram, 11)     → 100 ciclos
3. Computar a = b + c   → 0 ciclos (CPU)
4. set_ram(ram, 12, a)  → 100 ciclos
Total: 300 ciclos
```

**TP2 (Com Cache):**
```
Primeira execução (cache vazia):
1. ucm_access(ucm, 10, READ)   → MISS L1, L2, L3 → 161 ciclos
2. ucm_access(ucm, 11, READ)   → HIT L1 (mesmo bloco) → 1 ciclo
3. Computar a = b + c          → 0 ciclos
4. ucm_access(ucm, 12, WRITE)  → HIT L1 (mesmo bloco) → 161 ciclos (write-through)
Total: 323 ciclos (pior que TP1 neste caso!)

Segunda execução (dados em cache):
1. ucm_access(ucm, 10, READ)   → HIT L1 → 1 ciclo
2. ucm_access(ucm, 11, READ)   → HIT L1 → 1 ciclo
3. Computar                    → 0 ciclos
4. ucm_access(ucm, 12, WRITE)  → 161 ciclos (write-through)
Total:   163 ciclos (muito melhor!)
```

**Benefício:** Reutilização de dados (temporal locality).

---

## 3. **src/program.c**

### Modificação em Todas as Funções Existentes

#### **ANTES (TP1) - CPU recebe RAM:**
```c
void program_mult(RAM* ram, Register* reg, int multiplicand, int multiplier) {
  // ... instruções ...
  
  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ram, inst);  // ← Passa RAM
  }
}
```

#### **DEPOIS (TP2) - CPU recebe UCM:**
```c
void program_mult(RAM* ram, Register* reg, int multiplicand, int multiplier) {
  // ... instruções ... 
  
  // CRIAR UCM
  UCM* ucm = ucm_create(ram);
  
  while (reg->IR != HALT && reg->PC < MEMORY_SIZE) {
    execute_cpu(reg, ucm, inst);  // ← Passa UCM
  }
  
  // IMPRIMIR ESTATÍSTICAS
  printf("\n=== PROGRAM MULT STATISTICS ===\n");
  ucm_print_stats(ucm);
  
  // DESTRUIR UCM
  ucm_destroy(ucm);
}
```

**Mudanças:**
1. ✅ Criar UCM antes do loop
2. ✅ Passar UCM para `execute_cpu()`
3. ✅ Imprimir estatísticas após execução
4. ✅ Destruir UCM no final

---

### Programas Modificados (TP1 → TP2)

Todos os seguintes programas seguiram o mesmo padrão:

| Função | Descrição |
|--------|-----------|
| `program_mult()` | Multiplicação por soma repetida |
| `program_fibonacci()` | Sequência de Fibonacci |
| `program_sum_matrix()` | Soma de matrizes |
| `program_div()` | Divisão por subtração repetida |
| `program_fat()` | Fatorial |

---

### Novo Programa Adicionado (TP2)

#### `program_matrix_mult()` - Multiplicação de Matrizes

**Propósito:** Programa pesado para testar caches com muitos acessos à memória.

**Características:**
- **Não usa instruções da CPU** (acessa UCM diretamente)
- **Muitos acessos:** ~2×size³ leituras + size² escritas
- **Padrão irregular:** Acessa 3 matrizes diferentes

**Código:**
```c
void program_matrix_mult(RAM* ram, Register* reg, int size) {
  // Layout na RAM:
  // A:  RAM[0.. size²-1]
  // B:  RAM[size²..2×size²-1]
  // C: RAM[2×size²..3×size²-1]
  
  // Inicializa matrizes
  for (int i = 0; i < size * size; i++) {
    set_ram(ram, base_a + i, (i % 10) + 1);  // A[i] = 1.. 10
    set_ram(ram, base_b + i, (i % 10) + 1);  // B[i] = 1..10
    set_ram(ram, base_c + i, 0);             // C[i] = 0
  }
  
  UCM* ucm = ucm_create(ram);
  ucm_reset_stats(ucm);
  
  // Multiplicação:  C[i][j] = Σ(A[i][k] × B[k][j])
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      int sum = 0;
      
      for (int k = 0; k < size; k++) {
        int a_val = ucm_access(ucm, base_a + (i*size+k), UCM_READ, 0);
        int b_val = ucm_access(ucm, base_b + (k*size+j), UCM_READ, 0);
        sum += a_val * b_val;
      }
      
      ucm_access(ucm, base_c + (i*size+j), UCM_WRITE, sum);
    }
  }
  
  ucm_print_stats(ucm);
  ucm_destroy(ucm);
}
```

**Complexidade de Acessos:**
```
size = 10: 
- Leituras de A: 10 × 10 × 10 = 1000
- Leituras de B:  10 × 10 × 10 = 1000
- Escritas em C: 10 × 10 = 100
Total: 2100 acessos à memória
```

**Uso:**
```c
int main(void) {
  Register reg = {0, 0, 0, 0, 0};
  RAM* ram = create_empty_ram(MEMORY_SIZE);
  
  program_matrix_mult(ram, &reg, 10);  // Matrix 10×10
  
  destroy_ram(ram);
  return 0;
}
```

**Diferença dos Outros Programas:**
- `program_mult`, `program_fibonacci`, etc. → Usam instruções da CPU
- `program_matrix_mult` → Acessa UCM diretamente (mais controle)

---

## 4. **include/ram.h** - Funções Auxiliares Adicionadas

### Funções Inline Adicionadas

```c
// ADICIONADO NO TP2:
static inline size_t word_to_block(size_t word_address) {
  return word_address / WORDS_PER_BLOCK;
}

static inline size_t word_to_offset(size_t word_address) {
  return word_address % WORDS_PER_BLOCK;
}
```

**Propósito:** Converter endereço de palavra → (bloco, offset).

**Inline:** Função expandida no local de chamada (sem overhead de função).

**Uso:**
```c
int address = 10;
size_t block = word_to_block(address);   // = 10 / 4 = 2
size_t offset = word_to_offset(address);  // = 10 % 4 = 2
// Endereço 10 = Bloco 2, Offset 2
```

**Contexto:** Usadas por `ucm_read()` e `ucm_write()` para calcular bloco. 

---

## Resumo das Modificações

| Arquivo | Modificação | Propósito |
|---------|-------------|-----------|
| **Makefile** | `CFLAGS ? =` | Permitir sobrescrita com `-D` flags |
| **cpu.h** | `#include "ucm.h"` | Incluir UCM |
| **cpu.h** | `execute_cpu(... , UCM*, ...)` | Mudar assinatura |
| **cpu.c** | `ucm_access()` em todas operações | Usar hierarquia de cache |
| **program.c** | Criar/destruir UCM em cada programa | Integrar UCM |
| **program.c** | `program_matrix_mult()` nova | Teste pesado de cache |
| **ram.h** | `word_to_block()`, `word_to_offset()` | Funções auxiliares |

---

## Impacto das Modificações

### Performance
**TP1:**
- Todo acesso = 100 ciclos (RAM)
- Previsível, mas lento

**TP2:**
- Primeiro acesso = 161 ciclos (carrega cache)
- Acessos subsequentes = 1 ciclo (L1 hit)
- **Muito mais rápido em programas com reuso de dados**

### Complexidade
**TP1:**
- Simples: CPU ↔ RAM

**TP2:**
- Complexo: CPU ↔ UCM ↔ L1 ↔ L2 ↔ L3 ↔ RAM
- Mais realista (arquiteturas modernas)

### Estatísticas
**TP1:**
- Nenhuma métrica de performance

**TP2:**
- Hit rates de L1, L2, L3
- Total de hits/misses
- Tempo total e médio
- **Possibilita análise de desempenho**

---

## Exemplo Completo de Mudança

### TP1 - Programa de Multiplicação
```c
void program_mult(RAM* ram, Register* reg, int a, int b) {
  Instruction inst[100];
  // ... montar instruções ...
  
  while (reg->IR != HALT) {
    execute_cpu(reg, ram, inst);
  }
  
  // Sem estatísticas
}

void execute_cpu(Register* reg, RAM* ram, Instruction* memory) {
  // ...
  case ADD:
    reg->R1 = get_ram(ram, inst.optr1);      // 100 ciclos
    reg->R2 = get_ram(ram, inst.optr2);      // 100 ciclos
    reg->AC = reg->R1 + reg->R2;
    set_ram(ram, inst.optr3, reg->AC);       // 100 ciclos
    break;
  // ... 
}
```

**Tempo para 1 ADD:** 300 ciclos (sempre)

---

### TP2 - Programa de Multiplicação
```c
void program_mult(RAM* ram, Register* reg, int a, int b) {
  Instruction inst[100];
  // ... montar instruções ... 
  
  UCM* ucm = ucm_create(ram);               // ← CRIAR UCM
  
  while (reg->IR != HALT) {
    execute_cpu(reg, ucm, inst);            // ← PASSAR UCM
  }
  
  printf("\n=== STATISTICS ===\n");
  ucm_print_stats(ucm);                     // ← ESTATÍSTICAS
  
  ucm_destroy(ucm);                         // ← DESTRUIR UCM
}

void execute_cpu(Register* reg, UCM* ucm, Instruction* memory) {
  // ...
  case ADD:
    reg->R1 = ucm_access(ucm, inst. optr1, UCM_READ, 0);   // 1-161 ciclos
    reg->R2 = ucm_access(ucm, inst.optr2, UCM_READ, 0);   // 1-161 ciclos
    reg->AC = reg->R1 + reg->R2;
    ucm_access(ucm, inst.optr3, UCM_WRITE, reg->AC);      // 161 ciclos (write-through)
    break;
  // ...
}
```

**Tempo para 1 ADD:**
- Primeira vez: ~323 ciclos (miss)
- Próximas vezes: ~163 ciclos (hits em leitura)
- **Média muito menor em loops! **

---

## Compatibilidade

### Código Antigo (TP1) Funciona no TP2?
**Não** - assinatura de `execute_cpu()` mudou.

**Migração:**
```c
// TP1:
execute_cpu(reg, ram, inst);

// TP2 (mínimo):
UCM* ucm = ucm_create(ram);
execute_cpu(reg, ucm, inst);
ucm_destroy(ucm);
```

### RAM Ainda Existe?
**Sim! ** RAM é encapsulada pelo UCM, mas ainda é acessível: 

```c
UCM* ucm = ucm_create(ram);
// ... usar ucm ... 

// Ainda pode acessar RAM diretamente (bypass cache):
int val = get_ram(ram, 10);  // ← Funciona (mas não recomendado)
```

**Recomendação:** Use sempre `ucm_access()` para manter consistência. 