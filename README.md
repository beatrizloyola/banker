# Algoritmo do Banqueiro

Implementação em C do algoritmo do banqueiro (Silberschatz et al., *Operating System Concepts*, 10ª ed.), para a disciplina de Infraestrutura de Software. O número de clientes e o de tipos de recursos não são fixos: vêm dos arquivos de entrada e da linha de comando.

## Compilar e executar

```bash
make          # gera o executável ./banker
make clean    # remove o executável
```

Compila com `gcc -Wall -Wextra -std=c11`, sem warnings. Funciona em Linux, Unix e macOS.

```bash
./banker 10 5 7
```

Cada argumento é o total de instâncias de um tipo de recurso. A quantidade de argumentos define o número de recursos `m`. O exemplo acima tem três recursos, com 10, 5 e 7 instâncias.

O programa lê `customer.txt` e `commands.txt` **do diretório em que for executado** e escreve `result.txt` nesse mesmo diretório. O `result.txt` anterior é sobrescrito.

## Arquivos de entrada

### `customer.txt`

Uma linha por cliente, com o máximo que ele pode pedir de cada recurso. O número de linhas define o número de clientes `n`.

```
7,5,3
3,2,2
9,0,2
2,2,2
4,3,3
```

- Os números são separados **somente por vírgula**, sem espaços (`7,5,3`).
- Cada linha deve ter exatamente `m` valores, todos inteiros não negativos.
- Nenhum valor pode ser maior que o total do recurso correspondente (argumento da linha de comando).
- Linhas em branco são ignoradas. Fim de linha no estilo Windows (`\r\n`) é aceito.

### `commands.txt`

Uma instrução por linha, com os campos separados por espaço.

| Comando | Formato | Efeito |
|---|---|---|
| `RQ` | `RQ i r1 ... rm` | cliente `i` pede `r1..rm` instâncias |
| `RL` | `RL i r1 ... rm` | cliente `i` devolve `r1..rm` instâncias |
| `*` | `*` | imprime o estado atual (matrizes e `AVAILABLE`) |

- `i` deve estar entre `0` e `n-1`.
- `RQ` e `RL` precisam de exatamente `m` valores, todos inteiros não negativos.
- Linhas em branco são ignoradas.

## Saída (`result.txt`)

Uma linha por comando:

| Situação | Mensagem |
|---|---|
| pedido concedido | `Allocate to customer i the resources ...` |
| devolução feita | `Release from customer i the resources ...` |
| pedido acima do máximo declarado | `The customer i request ... was denied because exceed its maximum allocation` |
| pedido acima do disponível | `The resources <available> are not enough to customer i request ...` |
| pedido que levaria a estado inseguro | `The customer i request ... was denied because result in an unsafe state` |
| comando `*` | tabela `MAXIMUM \| ALLOCATION \| NEED` e a linha `AVAILABLE ...` |

A largura das colunas da tabela é calculada pelo conteúdo, então o alinhamento se mantém com qualquer `n`, `m` e valores de vários dígitos. Com `m = 3` e valores de um dígito, a saída é idêntica ao `result.txt` de exemplo do enunciado.

As verificações de um `RQ` seguem esta ordem: primeiro contra o `Need` do cliente, depois contra `Available`, e por fim o algoritmo de segurança sobre o estado simulado. Se o estado ficar inseguro, a alocação é desfeita e o estado volta ao que era antes do pedido.

## Tratamento de erros

O programa não trava com entrada inválida. Ele imprime uma mensagem em `stderr` e encerra com código `255`. Os casos tratados:

- Nenhum argumento, argumento não numérico, negativo ou grande demais.
- `customer.txt` ou `commands.txt` inexistente ou sem permissão de leitura.
- `customer.txt` vazio, ou com formato diferente de `7,5,3` (vírgula faltando ou duplicada, vírgula no fim, espaços, letras, sinais, valor negativo).
- Número de valores de uma linha de `customer.txt` diferente do número de argumentos.
- Valor máximo maior que o total do recurso.
- Em `commands.txt`: comando desconhecido, `id` fora do intervalo, número negativo ou quantidade de valores diferente de `m`.

`commands.txt` é validado **por inteiro antes de executar qualquer comando**, e `result.txt` só é criado depois disso. Assim, um erro de formato nunca deixa um `result.txt` parcial nem apaga o de uma execução anterior.

## Decisões de projeto

- **Devolução maior que a alocação** (`RL` com mais do que o cliente possui): a devolução é recusada, o estado não muda, uma mensagem vai para `stderr` e a execução continua. Nada é escrito no `result.txt` para esse comando.
- **`commands.txt` vazio** (ou só com linhas em branco): não é erro. O programa termina com código `0` e um `result.txt` vazio.
- **Formato de `customer.txt`:** foi seguido literalmente o exemplo do enunciado, então espaço ao redor da vírgula (`7, 5, 3`) é rejeitado.

## Estrutura do código

Tudo está em `banker.c`.

| Parte | Função |
|---|---|
| `bank_alloc` / `bank_free` | vetores e matrizes dinâmicos (`available`, `total`, `max`, `allocation`, `need`), com índice `i * m + j` |
| `parse_nonneg` | conversão com `strtol`, rejeitando lixo, negativo e valor grande demais |
| `count_lines`, `load_customers` | lê `customer.txt` (primeira passada conta `n`, segunda preenche as matrizes) |
| `parse_line`, `load_commands` | lê e valida `commands.txt` inteiro |
| `is_safe` | algoritmo de segurança, trabalhando só em cópias locais |
| `request_resources` | algoritmo de requisição (`RQ`), com simulação e rollback |
| `release_resources` | devolução (`RL`) |
| `print_state` | comando `*` |

## Testes

`evidencias.log` contém a gravação dos testes:

- o exemplo do enunciado, comparado com `diff` contra o `result.txt` esperado (idêntico);
- `valgrind --leak-check=full`: nenhum vazamento e nenhum erro de memória, inclusive em um caminho de erro;
- todos os casos de erro listados acima;
- execuções com tamanhos diferentes do exemplo (3 clientes × 2 recursos e 6 clientes × 4 recursos), para mostrar que nada é fixo no código.

## Limitações conhecidas

- As linhas dos arquivos de entrada são lidas com buffers de 256 bytes. Uma linha com 256 caracteres ou mais (na prática, `m` acima de cerca de 60 recursos) é cortada na leitura e dá erro de formato.
- Os arquivos `customer.txt` e `commands.txt` precisam estar no diretório atual. Não há argumento para indicar outro caminho.
