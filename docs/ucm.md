# Documentação - `ucm.h` e `ucm.c`

## **include/ucm.h**

### Enum `UCM_Operation`
```c
typedef enum {
  UCM_READ,   // Operação de leitura
  UCM_WRITE   // Operação de escrita
} UCM_Operation;
```

**Propósito:** Define os tipos de operação que podem ser realizadas na memória. 

**Valores:**
- `UCM_READ`: Leitura de dados
- `UCM_WRITE`: Escrita de dados

**Uso:**
```c
int value = ucm_access(ucm, 10, UCM_READ, 0);      // Lê endereço 10
ucm_access(ucm, 20, UCM_WRITE, 42);                 // Escreve 42 no endereço 20
```

---

### Estrutura `UCM`
```c
typedef struct UCM {
  Cache* L1;              // Cache L1 (mais rápida)
  Cache* L2;              // Cache L2 (intermediária)
  Cache* L3;              // Cache L3 (mais lenta)
  RAM* ram;               // Memória principal
  
  int global_time;        // Timestamp global (LRU)
  
  int total_accesses;     // Total de acessos à memória
  int total_hits;         // Total de hits (qualquer nível)
  int total_misses;       // Total de misses (foi à RAM)
  
  int total_time;         // Tempo total em ciclos
} UCM;
```

**Propósito:** **U**nified **C**ache **M**anager - gerencia toda a hierarquia de memória.

**Campos:**
- **Hierarquia:**
  - `L1`: Cache mais rápida (1 ciclo, pequena)
  - `L2`: Cache intermediária (10 ciclos, média)
  - `L3`: Cache mais lenta (50 ciclos, grande)
  - `ram`: Memória principal (100 ciclos, muito grande)

- **Controle:**
  - `global_time`: Contador de tempo para política LRU

- **Estatísticas:**
  - `total_accesses`: Quantas operações foram feitas
  - `total_hits`: Quantas foram encontradas em alguma cache
  - `total_misses`: Quantas precisaram ir à RAM
  - `total_time`: Tempo acumulado em ciclos

---

## **src/ucm.c**

### Defines Configuráveis
```c
#ifndef L1_SIZE
#define L1_SIZE 32
#endif

#ifndef L2_SIZE
#define L2_SIZE 64
#endif

#ifndef L3_SIZE
#define L3_SIZE 128
#endif
```

**Propósito:** Define tamanhos padrão das caches (podem ser sobrescritos via `-D` flags).

**Valores Padrão:**
- L1: 32 linhas
- L2: 64 linhas
- L3: 128 linhas

**Sobrescrita:**
```bash
gcc -DL1_SIZE=8 -DL2_SIZE=16 -DL3_SIZE=32 ... 
```

---

### `UCM* ucm_create(RAM* ram)`

**Propósito:** Cria e inicializa o gerenciador de cache unificado.

**Parâmetros:**
- `ram`: Ponteiro para a RAM

**Retorno:**
- Ponteiro para UCM criado
- `NULL` se erro

**Funcionamento:**
```
1. Valida RAM
2. Aloca estrutura UCM
3. Cria 3 caches:
   - L1: L1_SIZE linhas, 1 ciclo
   - L2: L2_SIZE linhas, 10 ciclos
   - L3: L3_SIZE linhas, 50 ciclos
4. Se alguma falhar:  destroi todas e retorna NULL
5. Inicializa estatísticas (tudo em 0)
6. Retorna UCM
```

**Uso:**
```c
RAM* ram = create_empty_ram(256);
UCM* ucm = ucm_create(ram);

if (ucm == NULL) {
  printf("Erro ao criar UCM\n");
  return -1;
}
```

**Hierarquia Criada:**
```
     CPU
      |
     L1 (1 ciclo, 32 linhas)
      |
     L2 (10 ciclos, 64 linhas)
      |
     L3 (50 ciclos, 128 linhas)
      |
    RAM (100 ciclos, 256 blocos)
```

---

### `void ucm_destroy(UCM* ucm)`

**Propósito:** Libera memória do UCM e todas as caches.

**Parâmetros:**
- `ucm`: UCM a destruir

**Funcionamento:**
```
1. Destroi L1
2. Destroi L2
3. Destroi L3
4. Libera estrutura UCM
```

**Nota:** NÃO destroi a RAM (ela é externa ao UCM).

**Uso:**
```c
ucm_destroy(ucm);
destroy_ram(ram);  // Separado! 
```

---

### `static void ucm_handle_miss(UCM* ucm, Cache* cache, int block_address, Block* block)`

**Propósito:** Carrega um bloco em uma cache específica após um miss.

**Parâmetros:**
- `ucm`: UCM
- `cache`: Cache onde carregar
- `block_address`: Endereço do bloco
- `block`: Dados do bloco

**Funcionamento:**
```
1. Chama cache_load()
2. Usa global_time para LRU
```

**Contexto:** Função auxiliar interna (static).

---

### `static int ucm_read(UCM* ucm, int address)`

**Propósito:** Realiza leitura na hierarquia de memória.

**Parâmetros:**
- `ucm`: UCM
- `address`: Endereço a ler

**Retorno:** Valor lido

**Algoritmo (Busca em Cascata):**
```
1. Incrementa global_time
2. Busca em L1:
   - HIT → retorna valor (tempo:  1 ciclo)
   - MISS → continua

3. Busca em L2:
   - HIT → carrega em L1, retorna valor (tempo: 1+10 ciclos)
   - MISS → continua

4. Busca em L3:
   - HIT → carrega em L2 e L1, retorna valor (tempo: 1+10+50 ciclos)
   - MISS → continua

5. Busca na RAM (CACHE MISS TOTAL):
   - Lê bloco da RAM
   - Carrega em L3, L2 e L1
   - Retorna valor (tempo: 1+10+50+100 = 161 ciclos)
```

**Política:** **Inclusive Cache** (dados propagam para cima).

**Exemplo - L2 Hit:**
```c
// Bloco 10 está na L2, mas não na L1

ucm_read(ucm, 40);  // address 40 = bloco 10, offset 0

// Fluxo:
// 1. Busca L1:  MISS (tempo += 1)
// 2. Busca L2: HIT!   (tempo += 10)
// 3. Carrega bloco 10 na L1
// 4. Retorna valor
// Total: 11 ciclos
```

**Exemplo - RAM Miss:**
```c
// Bloco 5 não está em nenhuma cache

ucm_read(ucm, 20);  // address 20 = bloco 5, offset 0

// Fluxo: 
// 1. Busca L1: MISS (tempo += 1)
// 2. Busca L2: MISS (tempo += 10)
// 3. Busca L3: MISS (tempo += 50)
// 4. Busca RAM (tempo += 100)
// 5. Carrega bloco 5 em L3, L2, L1
// 6. Retorna valor
// Total: 161 ciclos
```

---

### `static void ucm_write(UCM* ucm, int address, int value)`

**Propósito:** Realiza escrita na hierarquia de memória.

**Parâmetros:**
- `ucm`: UCM
- `address`: Endereço a escrever
- `value`: Valor a escrever

**Algoritmo (Write-Through):**
```
1. Incrementa global_time

2. Tenta atualizar L1:
   - HIT → atualiza valor na L1
   - MISS → carrega bloco da RAM, modifica, carrega na L1

3. Tenta atualizar L2 (se o bloco estiver lá):
   - HIT → atualiza valor na L2

4. Tenta atualizar L3 (se o bloco estiver lá):
   - HIT → atualiza valor na L3

5. SEMPRE escreve na RAM (write-through)

Tempo acumulado: L1 + L2 + L3 + RAM
```

**Política:** **Write-Through** (sempre escreve na RAM).

**Vantagem:** Consistência garantida (RAM sempre atualizada).  
**Desvantagem:** Toda escrita paga custo da RAM (100 ciclos).

**Exemplo - L1 Hit:**
```c
ucm_write(ucm, 10, 999);

// Fluxo:
// 1. Busca L1: HIT (tempo += 1)
//    - Atualiza L1[bloco 2, offset 2] = 999
// 2. Busca L2: HIT (tempo += 10)
//    - Atualiza L2 também
// 3. Busca L3: MISS (tempo += 50)
//    - Não atualiza (não está lá)
// 4. Escreve na RAM (tempo += 100)
//    - RAM[10] = 999
// Total: 161 ciclos
```

**Exemplo - L1 Miss (precisa carregar):**
```c
ucm_write(ucm, 50, 777);

// Fluxo:
// 1. Busca L1: MISS
//    - Busca bloco da RAM
//    - Modifica valor 777 no bloco
//    - Carrega bloco na L1
// 2. Busca L2: MISS (não atualiza)
// 3. Busca L3: MISS (não atualiza)
// 4. Escreve na RAM
// Total: L1 + L2 + L3 + RAM = 161 ciclos
```

---

### `int ucm_access(UCM* ucm, int address, UCM_Operation operation, int value)`

**Propósito:** Interface pública para acessar memória (leitura ou escrita).

**Parâmetros:**
- `ucm`: UCM
- `address`: Endereço
- `operation`: `UCM_READ` ou `UCM_WRITE`
- `value`: Valor a escrever (ignorado se READ)

**Retorno:**
- Valor lido (se READ)
- 0 (se WRITE)

**Funcionamento:**
```
1. Incrementa total_accesses
2. Se READ: chama ucm_read()
3. Se WRITE: chama ucm_write()
```

**Uso:**
```c
// Leitura
int val = ucm_access(ucm, 10, UCM_READ, 0);
printf("Valor:  %d\n", val);

// Escrita
ucm_access(ucm, 20, UCM_WRITE, 42);
```

---

### `void ucm_reset_stats(UCM* ucm)`

**Propósito:** Zera todas as estatísticas. 

**Parâmetros:**
- `ucm`: UCM

**Funcionamento:**
```
1. Zera estatísticas do UCM: 
   - global_time = 0
   - total_accesses = 0
   - total_hits = 0
   - total_misses = 0
   - total_time = 0

2. Zera estatísticas das caches:
   - cache_reset_stats(L1)
   - cache_reset_stats(L2)
   - cache_reset_stats(L3)
```

**Uso:**
```c
// Antes de um novo teste
ucm_reset_stats(ucm);

// Executa programa
program_mult(ram, &reg, 10, 10);

// Imprime estatísticas
ucm_print_stats(ucm);
```

---

### `double ucm_get_hit_rate(UCM* ucm)`

**Propósito:** Calcula taxa de hit global.

**Parâmetros:**
- `ucm`: UCM

**Retorno:** Hit rate (0.0 a 1.0)

**Cálculo:**
```c
hit_rate = total_hits / total_accesses
```

**Exemplo:**
```
total_hits = 85
total_accesses = 100
hit_rate = 85 / 100 = 0.85 (85%)
```

**Uso:**
```c
double hr = ucm_get_hit_rate(ucm);
printf("Hit Rate: %.2f%%\n", hr * 100.0);
```

---

### `void ucm_print_stats(UCM* ucm)`

**Propósito:** Imprime estatísticas detalhadas da hierarquia.

**Parâmetros:**
- `ucm`: UCM

**Saída:**
```
╔════════════════════════════════════════════════╗
║          MEMORY HIERARCHY STATISTICS          ║
╠════════════════════════════════════════════════╣
║ Total Memory Accesses:      95                  ║
║ Total Cache Hits:          93                  ║
║ Total Cache Misses:         2                  ║
╠════════════════════════════════════════════════╣
║ L1 Cache Statistics:                           ║
║   Hits:       93   Misses:       2              ║
║   Hit Rate:  97.89%                              ║
╠════════════════════════════════════════════════╣
║ L2 Cache Statistics:                            ║
║   Hits:        0   Misses:       2              ║
║   Hit Rate:  0.00%                              ║
╠════════════════════════════════════════════════╣
║ L3 Cache Statistics:                           ║
║   Hits:        0   Misses:       2              ║
║   Hit Rate:  0.00%                              ║
╠════════════════════════════════════════════════╣
║ Overall Hit Rate:  97.89%                        ║
║ Total Time (cycles):   5695                    ║
║ Average Time per Access:  59.95 cycles          ║
╚════════════════════════════════════════════════╝
```

**Informações Mostradas:**
- Total de acessos
- Hits e misses globais
- Estatísticas individuais de L1, L2, L3
- Hit rate global
- Tempo total e médio

---

## Fluxo de Uso Completo

### 1. Inicialização
```c
RAM* ram = create_empty_ram(256);
UCM* ucm = ucm_create(ram);

if (ucm == NULL) {
  fprintf(stderr, "Erro ao criar UCM\n");
  exit(1);
}
```

### 2. Reset Estatísticas
```c
ucm_reset_stats(ucm);
```

### 3. Operações de Memória
```c
// Escrever dados
ucm_access(ucm, 0, UCM_WRITE, 10);
ucm_access(ucm, 1, UCM_WRITE, 20);
ucm_access(ucm, 2, UCM_WRITE, 30);

// Ler dados
int a = ucm_access(ucm, 0, UCM_READ, 0);  // Hit provável
int b = ucm_access(ucm, 1, UCM_READ, 0);  // Hit provável
int c = ucm_access(ucm, 2, UCM_READ, 0);  // Hit provável

// Computação
int result = a + b + c;
ucm_access(ucm, 10, UCM_WRITE, result);
```

### 4. Mostrar Estatísticas
```c
ucm_print_stats(ucm);
```

### 5. Destruição
```c
ucm_destroy(ucm);
destroy_ram(ram);
```

---

## Políticas Implementadas

### 1. Substituição:  LRU (Least Recently Used)
**Onde:** Em cada cache individual  
**Como:** Usa `lru_counter` atualizado com `global_time`

```c
// Toda leitura/escrita atualiza LRU
line->lru_counter = ucm->global_time;
```

**Benefício:** Mantém dados mais recentemente usados. 

---

### 2. Inclusão: Inclusive Cache
**Onde:** Entre níveis de cache  
**Como:** Dados em L1 também estão em L2 e L3

```c
// Ao carregar da RAM:
ucm_handle_miss(ucm, ucm->L3, block_address, &ram_block);  // L3
ucm_handle_miss(ucm, ucm->L2, block_address, &ram_block);  // L2
ucm_handle_miss(ucm, ucm->L1, block_address, &ram_block);  // L1
```

**Benefício:** Consistência entre níveis.

---

### 3. Escrita:  Write-Through
**Onde:** Escritas  
**Como:** Atualiza todas as caches onde o bloco está + RAM

```c
// Escrita sempre vai à RAM
ucm_write(ucm, address, value);
// → Atualiza L1 (se presente)
// → Atualiza L2 (se presente)
// → Atualiza L3 (se presente)
// → SEMPRE escreve na RAM
```

**Vantagem:** RAM sempre consistente (seguro para crashes).  
**Desvantagem:** Toda escrita paga latência da RAM.

**Alternativa (não implementada):** Write-Back (escreve RAM só na substituição).

---

## Exemplo Completo:  Trace de Execução

### Programa: 
```c
ucm_access(ucm, 0, UCM_WRITE, 10);   // (1)
ucm_access(ucm, 1, UCM_WRITE, 20);   // (2)
ucm_access(ucm, 0, UCM_READ, 0);     // (3)
ucm_access(ucm, 100, UCM_READ, 0);   // (4)
```

### Trace:

#### **(1) WRITE endereço 0 = 10**
```
Bloco 0, offset 0

1. L1 search: MISS → carrega bloco 0 da RAM
2. L2 search: MISS
3. L3 search: MISS
4. RAM write: 0 = 10
Tempo: 1 + 10 + 50 + 100 = 161 ciclos

Estado: 
L1: [bloco 0]
L2: []
L3: []
```

#### **(2) WRITE endereço 1 = 20**
```
Bloco 0, offset 1

1. L1 search: HIT!  (bloco 0 já está)
   - Atualiza L1
2. L2 search: MISS
3. L3 search: MISS
4. RAM write: 1 = 20
Tempo: 1 + 10 + 50 + 100 = 161 ciclos

Estado:
L1: [bloco 0 com {10, 20, ?, ?}]
```

#### **(3) READ endereço 0**
```
Bloco 0, offset 0

1. L1 search: HIT! ✅
Tempo: 1 ciclo
Retorna: 10
```

#### **(4) READ endereço 100**
```
Bloco 25, offset 0

1. L1 search:  MISS
2. L2 search: MISS
3. L3 search: MISS
4. RAM read: bloco 25
   - Carrega em L3, L2, L1
Tempo: 1 + 10 + 50 + 100 = 161 ciclos

Estado final:
L1: [bloco 0, bloco 25, ...]
L2: [bloco 25]
L3: [bloco 25]
```

**Estatísticas Finais:**
```
Total accesses: 4
Total hits: 2 (operação 2 e 3)
Total misses: 2 (operação 1 e 4)
Total time: 161 + 161 + 1 + 161 = 484 ciclos
Average:  121 ciclos/acesso
Hit rate: 50%
```

---

## Tempos de Acesso

| Componente | Latência | Configurável?  |
|------------|----------|---------------|
| L1 | 1 ciclo | Sim (`access_time` no create) |
| L2 | 10 ciclos | Sim |
| L3 | 50 ciclos | Sim |
| RAM | 100 ciclos | Hardcoded (linha 120, 177) |

**Para mudar latência da RAM:**
```c
// Em ucm_read() linha 120:
access_time += 100;  // ← Mudar aqui

// Em ucm_write() linha 177:
access_time += 100;  // ← E aqui
```

---

## Melhorias Possíveis

### 1. Write-Back Policy
```c
// Ao invés de escrever RAM sempre: 
if (line->dirty) {
  // Escreve RAM só quando substituir
}
```

### 2. Exclusive Cache
```c
// Dados em L1 NÃO estão em L2
// Economiza espaço
```

### 3. Prefetching
```c
// Ao acessar bloco N, carrega N+1 também
// Beneficia acessos sequenciais
```

### 4. Configuração de Latências
```c
UCM* ucm_create_custom(RAM* ram, int l1_time, int l2_time, int l3_time, int ram_time);
```

---

## Resumo de Funções

| Função | Propósito | Público?  |
|--------|-----------|---------|
| `ucm_create()` | Cria hierarquia | Sim |
| `ucm_destroy()` | Libera memória | Sim |
| `ucm_access()` | Interface R/W | Sim ⭐ |
| `ucm_read()` | Lógica de leitura | Não (static) |
| `ucm_write()` | Lógica de escrita | Não (static) |
| `ucm_handle_miss()` | Carrega cache | Não (static) |
| `ucm_reset_stats()` | Zera estatísticas | Sim |
| `ucm_print_stats()` | Mostra estatísticas | Sim |
| `ucm_get_hit_rate()` | Calcula hit rate | Sim |

**Função principal:** `ucm_access()` - única necessária para uso normal. 

---

## Integração com Outros Módulos

```
┌─────────────────────────────────────┐
│            PROGRAMA                 │
│  (program_mult, program_matrix...)  │
└────────────┬────────────────────────┘
             │ ucm_access()
             ▼
┌─────────────────────────────────────┐
│              UCM                    │
│  - Coordena toda a hierarquia       │
│  - Implementa políticas             │
│  - Coleta estatísticas              │
└─┬─────────┬────────────┬────────────┘
  │         │            │
  ▼         ▼            ▼
┌───┐     ┌───┐       ┌───┐
│L1 │     │L2 │       │L3 │  (Cache)
└─┬─┘     └─┬─┘       └─┬─┘
  │         │            │
  └─────────┴────────────┘
             │
             ▼
         ┌───────┐
         │  RAM  │
         └───────┘
```

**UCM é o maestro que orquestra todo o sistema de memória. ** 🎵