# Diário de bugs

## Corrigidos

### 1. `main` seguia após falha de `bank_alloc`
- **Onde:** `main`, checagem de `bank_alloc()`.
- **Problema:** imprimia o erro mas não retornava. `bank_alloc` já tinha chamado `bank_free()`, então o loop do `argv` usava `total` e `available` liberados.
- **Correção:** `return -1;` depois do `fprintf`.

### 2. `main` terminava com `return 1`
- **Onde:** fim de `main`.
- **Problema:** execução bem-sucedida saía com código 1.
- **Correção:** `return 0;`.

### 3. Argumentos trocados no `fprintf` de `max > total`
- **Onde:** `load_customers`, `banker.c:88`.
- **Problema:** o texto diz `recurso %d na linha %d`, mas os argumentos eram `(lineno, col)`. Com `./banker 1 1 1` saía "recurso 1 na linha 0" em vez de "recurso 0 na linha 1".
- **Correção:** argumentos passados na ordem `col, lineno`.

## Abertos

Nenhum.
