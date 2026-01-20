# Documentação - `cache.h` e `cache.c`

## **include/cache.h**

### Estrutura `CacheLine`
```c
typedef struct CacheLine {
  int valid;              // Linha válida?  (1=sim, 0=não)
  int tag;                // Tag do bloco da RAM armazenado
  Block data;             // Dados do bloco (4 palavras)
  int lru_counter;        // Timestamp do último acesso (LRU)
} CacheLine;
```

**Propósito:** Representa uma linha individual da cache.

**Campos:**
- `valid`: Indica se a linha contém dados válidos (0=vazia, 1=ocupada)
- `tag`: Identifica qual endereço de bloco da RAM está armazenado
- `data`: Bloco de 4 palavras com os dados reais
- `lru_counter`: Timestamp usado pelo algoritmo LRU (Least Recently Used)

**Exemplo:**
```
valid=1, tag=5, data={10,20,30,40}, lru_counter=42
→ Esta linha contém o bloco 5 da RAM, acessado no tempo 42
```

---

### Estrutura `Cache`
```c
typedef struct Cache {
  CacheLine* lines;       // Array de linhas da cache
  int num_lines;          // Número de linhas
  int access_time;        // Tempo de acesso (ciclos)
  int hits;               // Contador de hits
  int misses;             // Contador de misses
} Cache;
```

**Propósito:** Representa uma cache completa (L1, L2 ou L3).

**Campos:**
- `lines`: Array dinâmico de linhas
- `num_lines`: Capacidade da cache (ex: L1=8, L2=16, L3=32)
- `access_time`: Latência de acesso em ciclos (L1=1, L2=10, L3=50)
- `hits`: Número de acessos bem-sucedidos
- `misses`: Número de acessos que falharam

**Exemplo:**
```
L1 Cache:  num_lines=8, access_time=1 ciclo
L2 Cache: num_lines=16, access_time=10 ciclos
L3 Cache:  num_lines=32, access_time=50 ciclos
```

---

## **src/cache.c**

### `Cache* cache_create(int num_lines, int access_time)`

**Propósito:** Cria e inicializa uma nova cache.

**Parâmetros:**
- `num_lines`: Número de linhas da cache
- `access_time`: Tempo de acesso em ciclos

**Retorno:**
- Ponteiro para a cache criada
- `NULL` se falha na alocação

**Funcionamento:**
```
1. Aloca memória para estrutura Cache
2. Aloca array de CacheLine
3. Inicializa todas as linhas: 
   - valid = 0 (vazia)
   - tag = -1 (sem bloco)
   - lru_counter = 0 (nunca acessada)
   - data zerado
4. Inicializa estatísticas (hits=0, misses=0)
```

**Uso:**
```c
Cache* l1 = cache_create(8, 1);    // L1: 8 linhas, 1 ciclo
Cache* l2 = cache_create(16, 10);  // L2: 16 linhas, 10 ciclos
Cache* l3 = cache_create(32, 50);  // L3: 32 linhas, 50 ciclos
```

---

### `void cache_destroy(Cache* cache)`

**Propósito:** Libera memória alocada pela cache.

**Parâmetros:**
- `cache`: Ponteiro para a cache

**Funcionamento:**
```
1. Libera array de linhas
2. Libera estrutura Cache
```

**Uso:**
```c
cache_destroy(l1);
cache_destroy(l2);
cache_destroy(l3);
```

---

### `CacheLine* cache_search(Cache* cache, int block_address, int word_offset)`

**Propósito:** Busca um bloco na cache.

**Parâmetros:**
- `cache`: Cache onde buscar
- `block_address`: Endereço do bloco procurado
- `word_offset`: Offset da palavra (não usado na busca, apenas para compatibilidade)

**Retorno:**
- Ponteiro para a linha se **HIT** (bloco encontrado)
- `NULL` se **MISS** (bloco não encontrado)

**Funcionamento:**
```
Para cada linha da cache:
  Se linha. valid == 1 E linha.tag == block_address:
    hits++
    Retorna linha  // HIT! 

misses++
Retorna NULL  // MISS!
```

**Uso:**
```c
CacheLine* line = cache_search(l1, 5, 0);
if (line != NULL) {
  // HIT:  bloco 5 está na cache
  int value = block_get_word(&line->data, 2);
} else {
  // MISS:  precisa buscar no próximo nível
}
```

**Efeitos Colaterais:**
- Incrementa `cache->hits` se encontrou
- Incrementa `cache->misses` se não encontrou

---

### `static int cache_find_lru_line(Cache* cache)`

**Propósito:** Encontra a linha para substituição usando política LRU.

**Parâmetros:**
- `cache`: Cache onde buscar

**Retorno:**
- Índice da linha a ser substituída

**Algoritmo:**
```
1. Primeira tentativa: procura linha vazia (valid=0)
   Se encontrar, retorna índice

2. Segunda tentativa: procura linha LRU (menor lru_counter)
   Percorre todas as linhas
   Encontra a com menor timestamp
   Retorna índice
```

**Características:**
- **Otimização:** Prioriza linhas vazias (não precisa substituir)
- **Política LRU:** Substitui a linha acessada há mais tempo

**Exemplo:**
```
Cache com 4 linhas: 
Linha 0: valid=1, lru_counter=10
Linha 1: valid=1, lru_counter=25
Linha 2: valid=1, lru_counter=5   ← Menor (LRU)
Linha 3: valid=1, lru_counter=15

Retorna:  2 (linha 2 será substituída)
```

---

### `void cache_load(Cache* cache, int block_address, const Block* block, int current_time)`

**Propósito:** Carrega um bloco da memória para a cache.

**Parâmetros:**
- `cache`: Cache destino
- `block_address`: Endereço do bloco
- `block`: Dados do bloco a carregar
- `current_time`: Timestamp atual (para LRU)

**Funcionamento:**
```
1. Encontra linha para substituir (LRU)
2. Marca linha como válida
3. Define tag = block_address
4. Copia dados do bloco
5. Atualiza lru_counter = current_time
```

**Uso:**
```c
Block ram_block;
get_ram_block(ram, 10, &ram_block);  // Busca bloco 10 da RAM

cache_load(l1, 10, &ram_block, 42);  // Carrega na L1
// Agora L1 contém uma cópia do bloco 10
```

**Contexto:** Chamada após um cache miss para trazer dados do nível inferior.

---

### `void cache_write(Cache* cache, int block_address, int word_offset, int value, int current_time)`

**Propósito:** Escreve um valor na cache (se o bloco estiver presente).

**Parâmetros:**
- `cache`: Cache alvo
- `block_address`: Endereço do bloco
- `word_offset`: Offset da palavra (0-3)
- `value`: Valor a escrever
- `current_time`: Timestamp atual

**Funcionamento:**
```
1. Busca o bloco na cache
2. Se encontrou (HIT):
   - Atualiza a palavra no bloco
   - Atualiza lru_counter
3. Se não encontrou (MISS):
   - Não faz nada (UCM deve carregar primeiro)
```

**Uso:**
```c
// Escrever valor 999 no offset 2 do bloco 5
cache_write(l1, 5, 2, 999, 50);
```

**Nota:** Política **write-through** é implementada na camada UCM (escreve em todas as caches + RAM).

---

### `void cache_reset_stats(Cache* cache)`

**Propósito:** Zera os contadores de estatísticas.

**Parâmetros:**
- `cache`: Cache a resetar

**Funcionamento:**
```
cache->hits = 0
cache->misses = 0
```

**Uso:**
```c
// Antes de um novo teste
cache_reset_stats(l1);
cache_reset_stats(l2);
cache_reset_stats(l3);
```

---

## Fluxo de Uso Completo

### 1. Criação da Hierarquia
```c
Cache* l1 = cache_create(8, 1);    // L1: pequena e rápida
Cache* l2 = cache_create(16, 10);  // L2: média
Cache* l3 = cache_create(32, 50);  // L3: grande e lenta
```

### 2. Leitura (Read Hit)
```c
// Tentar ler bloco 5
CacheLine* line = cache_search(l1, 5, 0);

if (line != NULL) {
  // HIT!  Bloco está na L1
  int value = block_get_word(&line->data, 2);
  printf("Valor: %d\n", value);
  line->lru_counter = global_time++;  // Atualiza LRU
}
```

### 3. Leitura (Read Miss → Carregar)
```c
// Tentar ler bloco 10
CacheLine* line = cache_search(l1, 10, 0);

if (line == NULL) {
  // MISS! Precisa buscar na L2/L3/RAM
  
  // (1) Buscar na RAM (ou nível inferior)
  Block ram_block;
  get_ram_block(ram, 10, &ram_block);
  
  // (2) Carregar na L1
  cache_load(l1, 10, &ram_block, global_time++);
  
  // (3) Agora pode ler
  line = cache_search(l1, 10, 0);
  int value = block_get_word(&line->data, 2);
}
```

### 4. Escrita
```c
// Escrever valor 42 no bloco 3, offset 1
cache_write(l1, 3, 1, 42, global_time++);
```

### 5. Substituição LRU
```c
// Cache cheia, precisa substituir
// Exemplo: L1 tem 2 linhas

// Estado inicial: 
// Linha 0: valid=1, tag=5, lru_counter=10
// Linha 1: valid=1, tag=7, lru_counter=25

// Carregar bloco 9 (não está na cache)
Block new_block;
get_ram_block(ram, 9, &new_block);

cache_load(l1, 9, &new_block, 30);

// Resultado:
// Linha 0: valid=1, tag=9, lru_counter=30  ← Substituiu (tinha menor LRU)
// Linha 1: valid=1, tag=7, lru_counter=25
```

### 6. Destruição
```c
cache_destroy(l1);
cache_destroy(l2);
cache_destroy(l3);
```

---

## Política de Substituição LRU

**Least Recently Used (LRU):** Substitui a linha acessada há mais tempo.

**Vantagem:** Mantém dados mais usados recentemente (temporal locality).

**Exemplo Temporal:**
```
Tempo 0: Carrega bloco 1 (lru_counter=0)
Tempo 5: Carrega bloco 2 (lru_counter=5)
Tempo 8: Acessa bloco 1 (lru_counter=8)  ← Atualizado! 
Tempo 10: Precisa substituir

Estado:
  Bloco 1: lru_counter=8
  Bloco 2: lru_counter=5  ← LRU (menor timestamp)

Substituição:  Remove bloco 2
```

---

## Estatísticas

### Hit Rate
```c
double hit_rate = (double)cache->hits / (double)(cache->hits + cache->misses);
```

**Exemplo:**
```
hits = 85
misses = 15
hit_rate = 85 / 100 = 0.85 (85%)
```

### Miss Rate
```c
double miss_rate = (double)cache->misses / (double)(cache->hits + cache->misses);
```

---

## Relação com UCM

A cache **NÃO** acessa outros níveis diretamente. Isso é responsabilidade do **UCM (Unified Cache Manager)**:

```
UCM coordena: 
  1. Buscar na L1 → cache_search(l1, ...)
  2. Se MISS, buscar na L2 → cache_search(l2, ...)
  3. Se MISS, buscar na L3 → cache_search(l3, ...)
  4. Se MISS, buscar na RAM → get_ram_block(...)
  5. Carregar em todos os níveis → cache_load(l1, .. .), cache_load(l2, .. .), etc.
```

**Cache é autônoma:** Só gerencia suas próprias linhas.  
**UCM é o orquestrador:** Gerencia a hierarquia completa.

---

## Exemplo Completo:  Simulação de 3 Acessos

```c
Cache* l1 = cache_create(2, 1);  // 2 linhas apenas
int time = 0;

// Acesso 1: Lê bloco 5
CacheLine* line = cache_search(l1, 5, 0);
if (!line) {  // MISS
  Block b5 = {{10, 20, 30, 40}};
  cache_load(l1, 5, &b5, time++);
}

// Estado L1:
// Linha 0: valid=1, tag=5, lru_counter=0
// Linha 1: valid=0

// Acesso 2: Lê bloco 10
line = cache_search(l1, 10, 0);
if (!line) {  // MISS
  Block b10 = {{50, 60, 70, 80}};
  cache_load(l1, 10, &b10, time++);
}

// Estado L1:
// Linha 0: valid=1, tag=5, lru_counter=0
// Linha 1: valid=1, tag=10, lru_counter=1

// Acesso 3: Lê bloco 5 novamente
line = cache_search(l1, 5, 0);  // HIT!
line->lru_counter = time++;

// Estado L1:
// Linha 0: valid=1, tag=5, lru_counter=2  ← Atualizado
// Linha 1: valid=1, tag=10, lru_counter=1

// Acesso 4: Lê bloco 15 (cache cheia!)
line = cache_search(l1, 15, 0);
if (!line) {  // MISS, precisa substituir
  Block b15 = {{90, 100, 110, 120}};
  cache_load(l1, 15, &b15, time++);
  // Substitui linha 1 (tag=10, lru_counter=1 é o menor)
}

// Estado final L1:
// Linha 0: valid=1, tag=5, lru_counter=2
// Linha 1: valid=1, tag=15, lru_counter=3  ← Substituiu bloco 10

// Estatísticas: 
// hits = 1 (acesso 3)
// misses = 3 (acessos 1, 2, 4)
// hit_rate = 1/4 = 25%
```

---

## Resumo de Funções

| Função | Propósito | Modifica Cache?  |
|--------|-----------|-----------------|
| `cache_create()` | Inicializa cache | Sim (aloca) |
| `cache_destroy()` | Libera memória | Sim (desaloca) |
| `cache_search()` | Busca bloco | Não (só incrementa hits/misses) |
| `cache_find_lru_line()` | Encontra linha LRU | Não (apenas consulta) |
| `cache_load()` | Carrega bloco | Sim (substitui linha) |
| `cache_write()` | Atualiza palavra | Sim (se HIT) |
| `cache_reset_stats()` | Zera estatísticas | Sim (reseta contadores) |