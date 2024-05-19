#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct memo_address
{
    int virtual_address;
    int page_number;
    int page_offset;
    int physical_address;
    int tlb_position;
    int pt_position;
    int value;

} memo_address;

void count_file_lines(char *line, FILE *ptr_file, int *lines);
void read_address(FILE *file, memo_address **address);
int check_tlb(memo_address **address, memo_address *tlb, int *tlb_hit);
void read_binary_file(const char *filename, memo_address *current_address);

int main(int argc, char **argv)
{
    int i;
    int tlb_hit = 0;
    int tlb_hit_rate;
    char *line = (char *)malloc(1000000 * sizeof(char));
    int lines = 0;
    int page_faults = 0;
    int page_fault_rate;

    memo_address *current_address = (memo_address *)malloc(sizeof(memo_address));
    memo_address *tlb = (memo_address *)malloc(16 * sizeof(memo_address));
    memo_address *pt = (memo_address *)malloc(128 * sizeof(memo_address));

    for (int i = 0; i < 16; i++)
    {
        tlb[i].page_number = -1;
    }

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

    for (i = 0; i < lines; i++)
    {
        read_address(ptr_file, &current_address);
        printf("\nVirtual address: %d ", current_address->virtual_address);
        int tlb_position = check_tlb(&current_address, tlb, &tlb_hit);
        if (tlb_position <= 15)
        {
            printf("TLB: %d ", current_address->tlb_position);
            current_address->physical_address = current_address->page_offset + ((current_address->pt_position) * 128);
            printf("Physical address: %d ", current_address->physical_address);
        }

        read_binary_file("BACKING_STORE.bin", current_address);
        printf("Value: %d", current_address->value);
    }

    page_fault_rate = page_faults / lines;
    tlb_hit_rate = tlb_hit / lines;

    printf("\nNumber of Translated Addresses = %d", lines);
    printf("\nPage Faults = %d", page_faults);
    printf("\nPage Fault Rate = %d", page_fault_rate);
    printf("\nTLB Hits = %d", tlb_hit);
    printf("\nTLB Hit Rate = %d", tlb_hit_rate);

    fclose(ptr_file);

    return 0;
}

void count_file_lines(char *line, FILE *ptr_file, int *lines)
{
    rewind(ptr_file);
    while (fgets(line, sizeof(char) * 1000000, ptr_file) != NULL)
    {
        (*lines)++;
    }
    rewind(ptr_file);
}

void read_address(FILE *file, memo_address **address)
{
    fscanf(file, "%d", &((*address)->virtual_address));
    (*address)->virtual_address = ((*address)->virtual_address) & 0xFFFF;
    (*address)->page_number = ((*address)->virtual_address) >> 8;
    (*address)->page_offset = ((*address)->virtual_address) & 0xFF;
    // printf("%d %d", (*address)->page_number, (*address)->page_offset );
}

int check_tlb(memo_address **address, memo_address *tlb, int *tlb_hit)
{
    int position = 0;
    for (position; position < 16; position++)
    {
        if ((*address)->page_number == (tlb[position]).page_number)
        {
            *tlb_hit += 1;
            return position;
        }
    }

    return position + 16;
}

void read_binary_file(const char *filename, memo_address *current_address) {
    FILE *binary_file = fopen(filename, "r");
    if (binary_file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    if (fseek(binary_file, current_address->virtual_address, SEEK_SET) != 0) {
        printf("Erro ao mover o ponteiro do arquivo.\n");
        return;
    }

    unsigned char buffer;
    if (fread(&buffer, sizeof(buffer), 1, binary_file) != 1) {
        printf("Erro ao ler o valor.\n");
        return;
    }

    // Verifique se o bit mais significativo está definido
    if (buffer & 0x80) {
        // Se o bit mais significativo estiver definido, o número é negativo
        // Calcule o complemento de dois
        current_address->value = (int)buffer - 0x100;
    } else {
        // Se o bit mais significativo não estiver definido, o número é positivo
        current_address->value = (int)buffer;
    }

    fclose(binary_file);
}