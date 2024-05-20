#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct memo_address
{
    int virtual_address;
    int page_number;
    int page_offset;
    int physical_address;
    int value;
    int memo_time;
    struct memo_address *next;
    struct memo_address *prev;

} memo_address;

void count_file_lines(char *line, FILE *ptr_file, int *lines);
void read_address(FILE *file, memo_address **address);
int check_list(memo_address *list, int page_number);
void read_binary_file(const char *filename, memo_address *current_address);
int len(memo_address *head);
void add_to_a_list(memo_address **list, memo_address **list_tail, memo_address *current, int index);
void remove_from_a_list(memo_address **list, memo_address **list_tail, int index);
void advance_memo_time(memo_address **tlb_head, memo_address **page_table_head, int tlb_len, int page_table_len);



int main(int argc, char **argv)
{
    int i, tlb_position, pt_position;
    int tlb_hit = 0;
    float tlb_hit_rate;
    char *line = (char *)malloc(1000000 * sizeof(char));
    int lines = 0;
    int page_faults = 0;
    float page_fault_rate;
    int tlb_len, page_table_len;

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
        tlb_len = len(tlb);
        if (tlb_position <= 15 && tlb_position >= 0)//TLB HIT
        {
            tlb_hit++;
            // current_address->tlb_position = tlb_position;
        }
        else //TLB MISS
        {
            pt_position = check_list(page_table, current_address->page_number);
            page_table_len = len(page_table);
            if (pt_position <= 127 && pt_position >= 0) //PAGE TABLE HIT
            {
                
            }
            else //PAGE FAULT
            {
                
                page_faults++;
            }
        }
        // printf("\n\tPT POSITION: %d\n", current_address->pt_position);

        // current_address->physical_address = current_address->page_offset + ((current_address->pt_position) * 256);
        read_binary_file("BACKING_STORE.bin", current_address);
        // printf("TLB: %d ", current_address->tlb_position);
        // printf("Physical address: %d ", current_address->physical_address);
        printf("Value: %d", current_address->value);
        advance_memo_time(&tlb, page_table, tlb_len, page_table_len);
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

int len(memo_address *head)
{
    int count = 0;
    while(head != NULL)
    {
        count++;
        head = head->next;      
    }
    return count; 
}

void add_to_a_list(memo_address **list, memo_address **list_tail, memo_address *current, int index)
{
    memo_address *new_node = (memo_address *)malloc(sizeof(memo_address));
    memcpy(new_node, current, sizeof(memo_address));
    new_node->memo_time = 0;
    new_node->next = NULL;
    new_node->prev = NULL;

    if(*list == NULL)
    {
        *list = new_node;
        *list_tail = new_node;
    }
    else
    {
        memo_address *temp = *list;
        for(int i = 0; i < index && temp->next != NULL; i++)
        {
            temp = temp->next;
        }

        new_node->next = temp->next;
        new_node->prev = temp;
        if(temp->next != NULL)
        {
            temp->next->prev = new_node;
        }
        temp->next = new_node;

        if(new_node->next == NULL)
        {
            *list_tail = new_node;
        }
    }
}


void remove_from_a_list(memo_address **list, memo_address **list_tail, int index)
{
    if(*list == NULL)
    {
        printf("A lista está vazia.\n");
        return;
    }

    memo_address *temp = *list;
    for(int i = 0; i < index && temp != NULL; i++)
    {
        temp = temp->next;
    }

    if(temp == NULL)
    {
        printf("Índice fora dos limites.\n");
        return;
    }

    if(temp->prev != NULL)
    {
        temp->prev->next = temp->next;
    }
    else
    {
        *list = temp->next;
    }

    if(temp->next != NULL)
    {
        temp->next->prev = temp->prev;
    }
    else
    {
        *list_tail = temp->prev;
    }

    free(temp);
}

void advance_memo_time(memo_address **tlb_head, memo_address **page_table_head, int tlb_len, int page_table_len)
{
    memo_address *aux = *tlb_head;
    
    for(int i = 0; i < tlb_len; i++)
    {
        aux->memo_time++;
    }
    memo_address *temp = *page_table_head;
    
    for(int i = 0; i < page_table_len; i++)
    {
        temp->memo_time++;
    }
}