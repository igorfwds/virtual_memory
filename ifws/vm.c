#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
    int p_lines;
    char line [100000];
    
    if (argc < 2)
    {
        printf("Por favor, forneça o nome do arquivo como argumento.\n");
        return 1;
    }

    FILE *ptr_file = fopen(argv[1], "r");
    if (ptr_file == NULL)
    {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    return 0;
}