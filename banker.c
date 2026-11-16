#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

int n, m;
int *available, *total;
int *max, *allocation, *need;

int count_lines(void){
    FILE *f = fopen("customer.txt", "r");
    if (f == NULL){
        fprintf(stderr, "Erro: falha ao ler arquivo de entrada.\n");
        return -1;
    }

    char line[256];
    int count = 0;
    while (fgets(line, sizeof line, f) != NULL){
        if (line[strspn(line, " \t\r\n")] != '\0'){
            count++;
        }
    }

    fclose(f);
    return count;
}

void bank_free(void){
    free(available);
    free(max);
    free(allocation);
    free(need);
    free(total);
}

int bank_alloc(void){
    total = calloc(m, sizeof(int));
    max = calloc(n * m, sizeof(int));
    allocation = calloc(n * m, sizeof(int));
    need = calloc(n * m, sizeof(int));
    available = calloc(m, sizeof(int));

    if (!available || !max || !allocation || !need || !total){
        bank_free();
        return -1;
    }

    return 0;
}

int parse_nonneg(const char *s, int *out){
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (end == s || *end != '\0' || errno != 0 || v < 0 || v > INT_MAX){
        return -1;
    }
    *out = (int)v;
    return 0;
}

int load_customers(void){
    FILE *f = fopen("customer.txt", "r");
    if (f == NULL){
        fprintf(stderr, "Erro: falha ao ler arquivo de entrada.\n");
        return -1;
    }

    char buf[256];
    int row = 0, lineno = 0;
    while (fgets(buf, sizeof buf, f) != NULL){
        lineno++;
        if (buf[strspn(buf, " \t\r\n")] == '\0'){
            continue;   // linha em branco
        }
        buf[strcspn(buf, "\r\n")] = '\0';

        int col = 0;
        char *p = buf;
        while (1){
            char *comma = strchr(p, ',');
            if (comma != NULL){
                *comma = '\0';
            }
            
            int v;
            if (col >= m || *p < '0' || *p > '9' || parse_nonneg(p, &v) != 0){
                fprintf(stderr, "Erro: linha %d inválida em customer.txt.\n", lineno);
                fclose(f);
                return -1;
            }
            if (v > total[col]){
                fprintf(stderr, "Erro: max excede o total do recurso %d na linha %d em customer.txt.\n", col, lineno);
                fclose(f);
                return -1;
            }
            max[row * m + col] = v;
            need[row * m + col] = v; // allocation é 0, então need = max
            col++;

            if (comma == NULL){
                break;
            }
            p = comma + 1;
        }
        if (col != m){
            fprintf(stderr, "Erro: linha %d deveria ter %d valores em customer.txt.\n", lineno, m);
            fclose(f);
            return -1;
        }
        row++;
    }

    fclose(f);
    return 0;
}

int less_or_equal(const int *a, const int *b){
    for (int j = 0; j < m; j++){
        if (a[j] > b[j]){
            return 0;
        }
    }
    return 1;
}

int is_safe(void){
// 1 é seguro, 0 é inseguro, -1 é falha de memória
    int *work = malloc(m * sizeof(int));
    int *finish = calloc( n, sizeof(int));
    if (!work || !finish){
        free(work);
        free(finish);
        fprintf(stderr, "Erro: falha na alocação de memória.\n");
        return -1;
    }
    memcpy(work, available, m * sizeof(int));

    // evita laço infinito. para quando uma passada inteira não acha ninguém
    int progress = 1;
    while (progress){
        progress = 0;
        for (int i = 0; i < n; i++){
            if(!finish[i] && less_or_equal(&need[i * m], work)){
                for (int j = 0; j < m; j++){
                    work[j] += allocation[i * m + j];
                }
                finish[i] = 1;
                progress = 1;
            }
        }
    }

    int safe = 1;
    for (int i = 0; i < n; i++){
        if(!finish[i]){
            safe = 0;
        }
    }

    free(work);
    free(finish);
    return safe;
}

void print_vec(const int *v){
    printf("%d", v[0]);
    for (int j = 1; j < m; j++){
        printf(" %d", v[j]);
    }
}

int request_resources(int i, const int *req){
// 0 concedido, -1 negado
    // 1. request <= need
    if(!less_or_equal(req, &need[i * m])){
        printf("The customer %d request ", i);
        print_vec(req);
        printf(" was denied because exceed its maximum allocation\n");
        return -1;
    }

    // 2. request <= available
    if(!less_or_equal(req, available)){
        printf("The resources ");
        print_vec(available);
        printf(" are not enough to customer %d request ", i);
        print_vec(req);
        printf(" \n");
        return -1;
    }

    // 3. simular
    for (int j = 0; j < m; j++){
        available[j] -= req[j];
        allocation[i * m + j] += req[j];
        need[i * m + j] -= req[j];
    }

    // 4. conferir se é seguro
    int safe = is_safe();
    if (safe == 1){
        printf("Allocate to customer %d the resources ", i);
        print_vec(req);
        printf("\n");
        return 0;
    }

    // 5. rollback
    for (int j = 0; j < m; j++){
        available[j] += req[j];
        allocation[i * m + j] -= req[j];
        need[i * m + j] += req[j];
    }
    if (safe == 0){
        printf("The customer %d request ", i);
        print_vec(req);
        printf(" was denied because result in an unsafe state\n");
    }
    return -1;
}

int release_resources(int i, const int *rel){
// 0 liberado, -1 negado
    // 1. release <= allocation
    if(!less_or_equal(rel, &allocation[i * m])){
        fprintf(stderr, "Erro: customer %d tentou liberar mais do que possui.\n", i);
        return -1;
    }

    // 2. atualizar estado
    for (int j = 0; j < m; j++){
        available[j] += rel[j];
        allocation[i * m + j] -= rel[j];
        need[i * m + j] += rel[j];
    }

    // 3. saída
    printf("Release from customer %d the resources ", i);
    print_vec(rel);
    printf("\n");
    return 0;
}

int print_vec_sp(const int *v){
// imprime e devolve quantos caracteres saíram
    int count = 0;
    for (int j =0; j < m; j++){
        count += printf("%d ", v[j]);
    }
    return count;
}

void pad(int n_chars, int width){
    for (int k = n_chars; k < width; k++){
        printf(" ");
    }
}

static int num_len(int x){
// quantidade de dígitos de x (x >= 0)
    int d = 1;
    while (x >= 10){
        x /= 10;
        d++;
    }
    return d;
}

static int vec_len(const int *v){
// tamanho de "a b c " (cada número + 1 espaço)
    int len = 0;
    for (int j = 0; j < m; j++){
        len += num_len(v[j]) + 1;
    }
    return len;
}

static int col_width(const int *mat, int minimo){
// largura da coluna é o maior entre o mínimo e a linha mais comprida
    int w = minimo;
    for (int i = 0; i < n; i++){
        int l = vec_len(&mat[i * m]);
        if (l > w){
            w = l;
        }
    }
    return w;
}

void print_state(void){
// mínimos 10 e 12 reproduzem o result.txt do professor com m = 3
    int w1 = col_width(max, 10);
    int w2 = col_width(allocation, 12);

    printf("%-*s|   %-*s|   NEED\n", w1, "MAXIMUM", w2, "ALLOCATION");
    for(int i = 0; i < n; i++){
        pad(print_vec_sp(&max[i * m]), w1);
        printf("|   ");
        pad(print_vec_sp(&allocation[i * m]), w2);
        printf("|   ");
        print_vec_sp(&need[i * m]);
        printf("\n");
    }
    printf("AVAILABLE ");
    print_vec_sp(available);
    printf("\n");
}

typedef enum {
    CMD_RQ,
    CMD_RL,
    CMD_STAR
} CmdType;

typedef struct {
    CmdType type;
    int id;
    int *vals; // m valores (RQ/RL), NULL para CMD_STAR
} Cmd;

// parseia uma linha, cmd->vals deve ter espaço para m ints
// 0 ok, 1 linha vazia, -1 erro de formato
int parse_line(char *buf, Cmd *cmd){
    const char *delim = " \t\r\n";
    char *tok = strtok(buf, delim);
    if (tok == NULL){
        return 1;
    }

    if (strcmp(tok, "*") == 0){
        if (strtok(NULL, delim) != NULL){
            return -1;
        }
        cmd->type = CMD_STAR;
        return 0;
    }

    if (strcmp(tok, "RQ") == 0){
        cmd->type = CMD_RQ;
    } else if (strcmp(tok, "RL") == 0){
        cmd->type = CMD_RL;
    } else {
        return -1;
    }

    // id do cliente
    tok = strtok(NULL, delim);
    if (tok == NULL || parse_nonneg(tok, &cmd->id) != 0 || cmd->id >= n){
        return -1;
    }

    // exatamente m valores
    for (int j = 0; j < m; j++){
        tok = strtok(NULL, delim);
        if (tok == NULL || parse_nonneg(tok, &cmd->vals[j]) != 0){
            return -1;
        }
    }
    if (strtok(NULL, delim) != NULL){
        return -1; // número a mais
    }
    return 0;
}

void free_commands(Cmd *cmds, int count){
    for (int i = 0; i < count; i++){
        free(cmds[i].vals);
    }
    free(cmds);
}

static int abort_load(FILE *f, Cmd *cmds, int cnt){
// limpeza de erro de load_commands (libera comandos aceitos e fecha o arquivo)
    free_commands(cmds, cnt);
    fclose(f);
    return -1;
}

int load_commands(const char *path, Cmd **out, int *count){
// lê e valida commands.txt inteiro antes de executar
// aloca *out (count comandos). 0 ok, -1 erro.
    FILE *f = fopen(path, "r");
    if (f == NULL){
        fprintf(stderr, "Erro: falha ao ler arquivo de comandos.\n");
        return -1;
    }

    Cmd *cmds = NULL;
    int cap = 0, cnt = 0, lineno = 0;
    char buf[256];

    while (fgets(buf, sizeof buf, f) != NULL){
        lineno++;
        if (cnt == cap){
            cap = cap ? cap * 2 : 16;
            Cmd *tmp = realloc(cmds, cap * sizeof(Cmd));
            if (tmp == NULL){
                return abort_load(f, cmds, cnt);
            }
            cmds = tmp;
        }

        Cmd *c = &cmds[cnt];
        c->vals = malloc(m * sizeof(int));
        if (c->vals == NULL){
            return abort_load(f, cmds, cnt);
        }

        int r = parse_line(buf, c);
        if (r == 1){
            free(c->vals);
            continue;
        }
        if (r < 0){
            fprintf(stderr, "Erro: linha %d inválida em commands.txt.\n", lineno);
            free(c->vals);
            return abort_load(f, cmds, cnt);
        }
        cnt++;
    }

    fclose(f);
    *out = cmds;
    *count = cnt;
    return 0;
}

int main(int argc, char *argv[]){
    if (argc < 2){
        fprintf(stderr, "Erro: quantidade de argumentos insuficiente.\n");
        return -1;
    }

    m = argc - 1;
    n = count_lines();
    if (n < 0){
        return -1;   // count_lines já imprimiu o erro
    }
    if (n == 0){
        fprintf(stderr, "Erro: arquivo vazio.\n");
        return -1;
    }

    if (bank_alloc() != 0){
        fprintf(stderr, "Erro: falha na alocação de memória.\n");
        return -1;
    }

    for (int j = 0; j < m; j++){
        if (parse_nonneg(argv[j + 1], &total[j]) != 0){
            fprintf(stderr, "Erro: argumento '%s' inválido.\n", argv[j + 1]);
            bank_free();
            return -1;
        }
        available[j] = total[j];
    }

    if (load_customers() != 0){
        bank_free();
        return -1;
    }

    Cmd *cmds = NULL;
    int ncmds = 0;
    if (load_commands("commands.txt", &cmds, &ncmds) != 0){
        bank_free();
        return -1;
    }

    // só abre result.txt depois que commands.txt foi validado
    if (freopen("result.txt", "w", stdout) == NULL){
        fprintf(stderr, "Erro: falha ao criar result.txt.\n");
        free_commands(cmds, ncmds);
        bank_free();
        return -1;
    }

    for (int k = 0; k < ncmds; k++){
        switch (cmds[k].type){
            case CMD_RQ:
                request_resources(cmds[k].id, cmds[k].vals);
                break;
            case CMD_RL:
                release_resources(cmds[k].id, cmds[k].vals);
                break;
            case CMD_STAR:
                print_state();
                break;
        }
    }

    free_commands(cmds, ncmds);
    bank_free();
    return 0;
}