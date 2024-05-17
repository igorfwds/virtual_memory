#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct memo_addresses{
    int virtual_address;
    int page_address;
    int physical_address;
    int tlb_address;
    memo_addresses *next;
    memo_addresses *prev;

}memo_addresses;

void count_file_lines(char *line, FILE *ptr_file, int *lines);
void read_addresses(FILE *file, int addressesLinesCount);
void add_to_list(memo_addresses **head, memo_addresses **tail, int virtual_address, int page_address, int physical_address, int tlb_address);

int main(int argc, char **argv){
    int address;
    char *line = (char*)malloc(1000000 * sizeof(char));
    int lines = 0;
    
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

    
    count_file_lines(line, ptr_file, &lines);

    read_addresses(ptr_file, lines);
    fclose(ptr_file);
    return 0;
}

void count_file_lines(char *line, FILE *ptr_file, int *lines){
    rewind(ptr_file);
    while (fgets(line, sizeof(char) * 1000000, ptr_file) != NULL)
    { 
        (*lines)++;
    }
    rewind(ptr_file);
}

void read_addresses(FILE *file, int addressesLinesCount){
    int address[addressesLinesCount];
    int pageNumber, pageOffset ;
    for (int i = 0; i < addressesLinesCount; i++){
        fscanf(file, "%d", &address[i]);
        address[i] = address[i] & 0xFFFF; // Mantém apenas os 16 bits menos significativos
    }

    // Agora você pode usar os números em 'address' no seu código
    // Por exemplo, para obter o número da página e o offset da página:
    for (int i = 0; i < addressesLinesCount; i++){
        pageNumber = address[i] >> 8; // Os 8 bits mais significativos são o número da página
        pageOffset = address[i] & 0xFF; // Os 8 bits menos significativos são o offset da página
        // Agora você pode usar 'pageNumber' e 'pageOffset' no seu código
        printf("\n %d) %d %d", i, pageNumber, pageOffset);
    }
    
}

void add_to_list(memo_addresses **head, memo_addresses **tail, int virtual_address, int page_address, int physical_address, int tlb_address) {
    memo_addresses *new_node = malloc(sizeof(memo_addresses));
    if (new_node == NULL) {
        printf("Erro ao alocar memória para novo nó.\n");
        return;
    }

    new_node->virtual_address = virtual_address;
    new_node->page_address = page_address;
    new_node->physical_address = physical_address;
    new_node->tlb_address = tlb_address;
    new_node->next = NULL;

    if (*head == NULL) {
        new_node->prev = NULL;
        *head = new_node;
    } else {
        new_node->prev = *tail;
        (*tail)->next = new_node;
    }
    *tail = new_node;
}