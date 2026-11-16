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

        int col = 0;
        for (char *tok = strtok(buf, " ,\t\r\n"); tok; tok = strtok(NULL, " ,\t\r\n")){
            int v;
            if (col >= m || parse_nonneg(tok, &v) != 0){
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

int main(int argc, char *argv[]){
    if (argc < 2){
        fprintf(stderr, "Erro: quantidade de argumentos insuficiente.\n");
        return -1;
    }

    m = argc - 1;
    n = count_lines();
    if (n <= 0){
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

    // etapas seguintes

    bank_free();
    return 0;
}