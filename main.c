#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int n, m;
int *available;
int *max, *allocation, *need;

int count_lines(){
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
}

int bank_alloc(void){
    available = calloc(m, sizeof(int));
    max = calloc(n * m, sizeof(int));
    allocation = calloc(n * m, sizeof(int));
    need = calloc(n * m, sizeof(int));

    if (!available || !max || !allocation || !need){
        bank_free();
        return -1;
    }

    return 0;
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
    }

    // etapas seguintes

    bank_free();
    return 1;
}