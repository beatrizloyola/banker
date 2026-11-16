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

### 4. `print_vec` imprimia o primeiro número duas vezes
- **Onde:** `print_vec`, Etapa 5.
- **Problema:** `printf("%d", v[0])` antes do laço, e o laço começava em `j = 0`. O vetor `1 0 2` saía `1 1 0 2`.
- **Correção:** laço começando em `j = 1`.

### 5. Sinal errado em `allocation` na simulação de RQ
- **Onde:** `request_resources`, passos 3 e 5.
- **Problema:** a simulação fazia `allocation -= req` e o rollback `allocation += req`, o inverso do certo. A alocação ficava negativa e `is_safe` negava tudo como "unsafe". Em execução real corromperia o estado.
- **Correção:** simulação com `+=` e rollback com `-=`.

### 6. `&rel` em vez de `rel` em `release_resources`
- **Onde:** `banker.c:216`, `less_or_equal(&rel, ...)`.
- **Problema:** `rel` já é ponteiro. `&rel` é o endereço da variável local, tipo `const int **`. A comparação lê lixo da pilha e recusa a liberação válida: `available` e `need` não mudam.
- **Correção:** `less_or_equal(rel, &allocation[i * m])`.

### 7. Aspas no lugar errado no `fprintf` de erro de RL
- **Onde:** `banker.c:217`.
- **Problema:** `"...possui.\n, i"` deixou o `, i` dentro da string. O `%d` fica sem argumento (comportamento indefinido) e a saída traz `, i` literal.
- **Correção:** `"...possui.\n", i`.

### 8. `customer.txt` aceita separador que não seja vírgula
- **Onde:** `load_customers`, `strtok(buf, " ,\t\r\n")`.
- **Problema:** `7 5,3`, `7 5 3`, `7,,5,3` e `7,5,3,` são aceitos sem erro e o programa roda normal. O enunciado manda recusar formatação diferente da indicada (vírgulas, ex.: `7,5,3`).
- **Correção:** `strchr(p, ',')` em vez de `strtok`. O campo precisa começar com dígito, então vazio, espaço e sinal são recusados. Só `\r\n` no fim da linha é descartado.

## Abertos

### 9. Mensagem dupla e incoerente quando `customer.txt` não abre
- **Onde:** `main`, após `count_lines()`.
- **Problema:** `count_lines` devolve -1 depois de imprimir "falha ao ler arquivo de entrada", e `main` imprime também "arquivo vazio". Vale para arquivo inexistente e sem permissão.
- **Correção:** em `main`, tratar `n < 0` (erro já impresso) separado de `n == 0` ("arquivo vazio").
