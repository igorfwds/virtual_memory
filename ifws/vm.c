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
    struct memo_address *next;
    struct memo_address *prev;

} memo_address;

void count_file_lines(char *line, FILE *ptr_file, int *lines);
void read_address(FILE *file, memo_address **address);
void add_to_tlb(memo_address **tlb, memo_address **tail_tlb, int page_number, memo_address **current);
void remove_from_tlb(memo_address **tlb, memo_address **tail_tlb);
void add_to_page_table(memo_address **pt, memo_address **tail_pt, int page_number, memo_address **current);
void remove_from_page_table(memo_address **pt, memo_address **tail_pt, int position);
int check_list(memo_address *list, int page_number);
void read_binary_file(const char *filename, memo_address *current_address);

int main(int argc, char **argv)
{
    int i, tlb_position, pt_position;
    int tlb_hit = 0;
    float tlb_hit_rate;
    char *line = (char *)malloc(1000000 * sizeof(char));
    int lines = 0;
    int page_faults = 0;
    float page_fault_rate;

    memo_address *current_address = (memo_address *)malloc(sizeof(memo_address));
    memo_address *tlb = NULL;
    memo_address *tlb_tail = NULL;
    memo_address *page_table = NULL;
    memo_address *page_table_tail = NULL;

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
        tlb_position = check_list(tlb, current_address->page_number);
        if (tlb_position <= 15 && tlb_position >= 0)//TLB HIT
        {
            tlb_hit++;
            current_address->tlb_position = tlb_position;
        }
        else //TLB MISS
        {
            pt_position = check_list(page_table, current_address->page_number);
            if (pt_position <= 127 && pt_position >= 0) //PAGE TABLE HIT
            {
                add_to_tlb(&tlb, &tlb_tail, current_address->page_number, &current_address);
                current_address->pt_position = page_table_tail->pt_position ;
                current_address->tlb_position = page_table_tail->tlb_position ;
                if (current_address->tlb_position > 15)
                {
                    remove_from_tlb(&tlb, &tlb_tail);
                }
            }
            else //PAGE FAULT
            {
                add_to_page_table(&page_table, &page_table_tail, current_address->page_number, &current_address);
                if(current_address->pt_position > 127)
                {
                    remove_from_page_table(&page_table, &page_table_tail, 0);
                }
                add_to_tlb(&tlb, &tlb_tail, current_address->page_number, &current_address);
                if (current_address->tlb_position > 15)
                {
                    remove_from_tlb(&tlb, &tlb_tail);
                }
                page_faults++;
            }
        }
        // printf("\n\tPT POSITION: %d\n", current_address->pt_position);

        current_address->physical_address = current_address->page_offset + ((current_address->pt_position) * 256);
        read_binary_file("BACKING_STORE.bin", current_address);
        printf("TLB: %d ", current_address->tlb_position);
        printf("Physical address: %d ", current_address->physical_address);
        printf("Value: %d", current_address->value);
    }
    fclose(ptr_file);

    page_fault_rate = ((float)page_faults/lines);
    tlb_hit_rate = ((float)tlb_hit/lines);

    printf("\nNumber of Translated Addresses = %d", lines);
    printf("\nPage Faults = %d", page_faults);
    printf("\nPage Fault Rate = %.3f", page_fault_rate);
    printf("\nTLB Hits = %d", tlb_hit);
    printf("\nTLB Hit Rate = %.3f", tlb_hit_rate);


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

void add_to_tlb(memo_address **tlb, memo_address **tail_tlb, int page_number, memo_address **current)
{
    memo_address *new_node = (memo_address *)malloc(sizeof(memo_address));
    new_node->page_number = page_number;
    new_node->next = NULL;
    if (*tail_tlb != NULL)
    {
        new_node->tlb_position = (*tail_tlb)->tlb_position + 1;
        (*tail_tlb)->next = new_node;
        new_node->prev = *tail_tlb;
    }
    else
    {
        new_node->tlb_position = 0;
        *tlb = new_node;
    }
    *tail_tlb = new_node;
    new_node->next = NULL;
    (*current)->tlb_position = new_node->tlb_position;
}

void remove_from_tlb(memo_address **tlb, memo_address **tail_tlb)
{
    if (*tlb == NULL)
    {
        printf("TLB está vazia.\n");
        return;
    }

    memo_address *temp = *tlb;
    *tlb = (*tlb)->next;
    if (*tlb != NULL)
    {
        (*tlb)->prev = NULL;
    }
    else
    {
        *tail_tlb = NULL;
    }
    free(temp);
    
    memo_address *aux = *tlb;
    while (aux != NULL)
    {
        (aux)->tlb_position--;
        aux = (aux)->next;
    }
}
void add_to_page_table(memo_address **pt, memo_address **tail_pt, int page_number, memo_address **current)
{
    memo_address *new_node = (memo_address *)malloc(sizeof(memo_address));
    new_node->page_number = page_number;
    new_node->next = NULL;
    new_node->prev = *tail_pt;

    if(new_node != NULL)
    {
        if(*pt == NULL)
        {
            new_node->pt_position = 0;
            *pt = new_node;
            (*tail_pt) = new_node;
            new_node->prev = NULL;
        }
        else
        {
            new_node->pt_position = (*tail_pt)->pt_position + 1;
            (*tail_pt)->next = new_node;
            new_node->prev = *tail_pt;
            (*tail_pt) = new_node;
        }
        new_node->next = NULL;
    }
    (*current)->pt_position = new_node->pt_position;
}

void remove_from_page_table(memo_address **pt, memo_address **tail_pt, int position)
{
    if (*pt == NULL)
    {
        printf("A tabela de páginas está vazia.\n");
        return;
    }

    memo_address *temp = *pt;

    if (position == 0)
    {
        *pt = temp->next;
        if (*pt != NULL)
        {
            (*pt)->prev = NULL;
        }
        else
        {
            *tail_pt = NULL;
        }
        free(temp);
    }
    else
    {
        for (int i = 0; temp != NULL && i < position - 1; i++)
        {
            temp = temp->next;
        }

        if (temp == NULL || temp->next == NULL)
        {
            return;
        }

        memo_address *next = temp->next->next;

        free(temp->next);

        temp->next = next;

        if (next != NULL)
        {
            next->prev = temp;
        }
        else
        {
            *tail_pt = temp;
        }
    }

    memo_address *aux = *pt;
    while (aux != NULL) {
        aux->pt_position--;
        aux = aux->next;
    }
}

int check_list(memo_address *list, int page_number)
{
    memo_address *current = list;
    int position = 0;
    while (current != NULL)
    {
        if (current->page_number == page_number)
        {
            return position;
        }
        position++;
        current = current->next;
    }
    return -1;
}

void read_binary_file(const char *filename, memo_address *current_address)
{
    FILE *binary_file = fopen(filename, "r");
    if (binary_file == NULL)
    {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    if (fseek(binary_file, current_address->virtual_address, SEEK_SET) != 0)
    {
        printf("Erro ao mover o ponteiro do arquivo.\n");
        return;
    }

    unsigned char buffer;
    if (fread(&buffer, sizeof(buffer), 1, binary_file) != 1)
    {
        printf("Erro ao ler o valor.\n");
        return;
    }

    if (buffer & 0x80)
    {
        current_address->value = (int)buffer - 0x100;
    }
    else
    {
        current_address->value = (int)buffer;
    }

    fclose(binary_file);
}