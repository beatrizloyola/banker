# Planejamento — Algoritmo do Banqueiro

## O que o enunciado pede
- Implementar em C o algoritmo do Banqueiro; `make` gera o executável `banker`.
- Chamada: `./banker 10 5 7`. Cada argumento é o total de instâncias de um recurso, então **m = argc-1**.
- `customer.txt`: uma linha por cliente (`7,5,3`), então **n = nº de linhas**. Cada linha é o Max do cliente.
- `commands.txt`: comandos `RQ i r1..rm`, `RL i r1..rm` e `*`.
- Saída em `result.txt`.
- O programa não pode crashar. Valida tudo e sai com mensagem de erro coerente.
- Nota: implementação 80%, relatório 10%, tratamento de erro 8%, README 2%.
  
## Etapas

### Etapa 1 — Estruturas de dados dinâmicas
- Declarar `n`, `m`, `available[m]`, `max[n][m]`, `allocation[n][m]`, `need[n][m]`.
- n e m só são conhecidos em runtime: usar `malloc`/`calloc` (vetor de ponteiros ou array 1D com `i*m+j`).
- Função `free_all()` e ponto único de saída, para não vazar memória em caminhos de erro.
- **Checkpoint:** compila com `-Wall -Wextra` sem warnings.

### Etapa 2 — Argumentos de linha de comando
- Exigir `argc >= 2`.
- Converter com `strtol`, não `atoi`. Rejeitar lixo (`abc`), negativos e overflow.
- Guardar em `total[m]` e inicializar `available = total`.

### Etapa 3 — Parser de `customer.txt`
- `fopen` com tratamento de falha.
- Primeira passada para contar linhas (n). Depois ler e validar cada linha.
- Validar:
  - exatamente m inteiros separados por vírgula por linha;
  - valores não negativos;
  - `max[i][j] <= total[j]` (exigência do livro);
  - arquivo não vazio; decidir o que fazer com linhas em branco no fim.
- Calcular `need = max - allocation` (allocation começa zerada).
- **Checkpoint:** com os exemplos, `need` igual a `max`.

### Etapa 4 — Algoritmo de segurança (coração)
`int is_safe(void)`, conforme o livro:
1. Copiar `available` para `work`. Criar `finish[n] = false`.
2. Procurar `i` com `!finish[i]` e `need[i] <= work`.
3. Ao achar: `work += allocation[i]`, `finish[i] = true`, repetir.
4. Sem mais candidatos: seguro se todos `finish` forem true.

- **Não alterar** o estado real. Usar só cópias locais.
- Teste isolado: estado do exemplo do livro (`available` 3 3 2) deve ser seguro, sequência T1,T3,T4,T2,T0.

### Etapa 5 — Algoritmo de requisição (RQ)
Ordem das verificações define a mensagem:
1. `request > need`: `The customer i request ... was denied because exceed its maximum allocation`.
2. `request > available`: `The resources <available> are not enough to customer i request <req>`.
3. Simular: `available -= req`, `allocation += req`, `need -= req`.
4. Se `is_safe()`: `Allocate to customer i the resources ...`.
5. Se inseguro: **rollback** do estado e `The customer i request ... was denied because result in an unsafe state`.

A mensagem "not enough" imprime o **available atual**.

### Etapa 6 — Liberação (RL)
- Validar id do cliente.
- Validar `release <= allocation`. O enunciado não define o caso contrário; sugestão: mensagem de erro e continuar, sem crashar.
- `available += rel`, `allocation -= rel`, `need += rel`.
- Saída: `Release from customer i the resources ...`.

### Etapa 7 — Parser e loop de `commands.txt`
- `fgets` linha a linha, tokenizar com `strtok` ou `sscanf`.
- Primeiro token: `RQ`, `RL` ou `*`. Outro valor é erro de formatação.
- RQ e RL: exatamente **1 + m** inteiros (id + m recursos). Isso cobre a validação de nº de recursos do enunciado.
- Validar id em `[0, n)`.
- **Decisão:** validar o arquivo inteiro antes de executar (sugerido), para nunca sair um `result.txt` parcial por erro de formato.

### Etapa 8 — Comando `*` e formato do `result.txt`
- Imprime a tabela `MAXIMUM | ALLOCATION | NEED` e a linha `AVAILABLE ...`.
- Copiar o espaçamento **exato** do `result.txt` (há espaços no fim de algumas linhas).
- Comparar com `diff -b`, ou replicar o espaçamento fielmente.
- Guardar cópia do `result.txt` do professor como `esperado.txt` antes de rodar, pois o programa sobrescreve o arquivo.

### Etapa 9 — Tratamento de erro (8%)
Casos a testar:
- Sem argumentos, ou argumento não numérico.
- Nº de argumentos diferente do nº de colunas do `customer.txt`.
- Nº de números nos comandos diferente de m.
- `customer.txt` ou `commands.txt` inexistente ou sem permissão.
- Arquivo vazio, vírgula faltando, letra no meio.
- Comando desconhecido, id fora do intervalo, número negativo.
- Max maior que o total de instâncias.

### Etapa 10 — Testes e evidências
- Rodar o exemplo e comparar com `diff` contra o gabarito.
- `valgrind --leak-check=full ./banker 10 5 7`: zero erros.
- Rodar todos os casos de erro da etapa 9.
- Capturar tudo em `evidencias.log` (`script` ou redirecionamento).
- Casos extras com n e m diferentes (ex.: 3×2, 6×4) para provar que nada é fixo no código.

### Etapa 11 — Entrega
- **Makefile:** alvos `all` (gera `banker`) e `clean`; flags `-Wall -Wextra -std=c11`.
- **README:** compilar, rodar, formato dos arquivos, decisões tomadas.
- **Relatório PDF (10%):** algoritmo, estruturas de dados, decisões, prints de teste, nota sobre o erro de digitação do PDF.
- **GitHub:** código, Makefile, README, relatório e `evidencias.log`. Não enviar os arquivos do professor.
- **Classroom:** PDF + `.tar` (código e Makefile). Nomes `login.pdf` e `login.tar`.
- Prazo: o enunciado só cita a data para alunos com laudo (quarta, 18/11). Confirmar o prazo geral com o professor.

## Próximo passo
Começar pelas etapas 1 e 2 (alocação dinâmica e argumentos) e mostrar o código para revisão antes de seguir ao parser.
