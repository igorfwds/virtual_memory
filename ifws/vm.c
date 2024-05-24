#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct memory_address
{
    int virtual_address;
    int page_number;
    int page_offset;
    int physical_address;
    int value;
    int time_in_memory;
    struct memory_address *next;
    struct memory_address *prev;

} memory_address;

void count_file_lines(char *line, FILE *ptr_file, int *lines);
void read_address(FILE *file, memory_address **address);
int check_list(memory_address *list, int page_number);
void read_binary_file(const char *filename, memory_address *current_address);
int len(memory_address *head);
void append(memory_address **list, memory_address **list_tail, memory_address *current);
void FIFO_replacement(memory_address **list, memory_address **list_tail, memory_address *current, int *fifo_index);
void LRU_replacement(memory_address **list, memory_address **list_tail, memory_address *current, int *LRU_index);
void reset_time_in_memory(memory_address **list, int index);
void update_time_in_memory(memory_address **list);
void find_next_LRU_index(memory_address **list, int *LRU_index);

int main(int argc, char **argv)
{
    int i;
    int tlb_hit = 0;
    float tlb_hit_rate;
    char *line = (char *)malloc(1000000 * sizeof(char));
    int lines = 0;
    int page_faults = 0;
    float page_fault_rate;
    int tlb_len, page_table_len;
    int fifo_index_tlb = 0;
    int fifo_index_page_table = 0;
    int LRU_index_page_table = 0;

    int *tlb_position = (int *)malloc(sizeof(int));
    int *page_table_position = (int *)malloc(sizeof(int));
    int debug_PT_hit = 0;
    int tamanho_PT = 0;

    memory_address *current_address = (memory_address *)malloc(sizeof(memory_address));
    if (current_address == NULL)
    {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    memory_address *tlb = NULL;
    memory_address *tlb_tail = NULL;
    memory_address *page_table = NULL;
    memory_address *page_table_tail = NULL;

    if (argc < 2)
    {
        printf("Por favor, forneça os parametros como argumento.\n");
        return 1;
    }

    FILE *ptr_file = fopen(argv[1], "r");
    FILE *output_file = fopen("correct.txt", "w");

    if (ptr_file == NULL)
    {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    count_file_lines(line, ptr_file, &lines);
    for (i = 0; i < lines; i++)
    {

        read_address(ptr_file, &current_address);
        fprintf(output_file, "Virtual address: %d ", current_address->virtual_address);

        *tlb_position = check_list(tlb, current_address->page_number);
        tlb_len = len(tlb);
        if (*tlb_position != -1) // TLB HIT
        {
            tlb_hit++;
            fprintf(output_file, "TLB: %d ", *tlb_position);
            reset_time_in_memory(&tlb, *tlb_position);
            *page_table_position = check_list(page_table, current_address->page_number);
        }
        else // TLB MISS
        {
            *page_table_position = check_list(page_table, current_address->page_number);
            page_table_len = len(page_table);
            if (*page_table_position != -1) // PAGE TABLE HIT
            {
                debug_PT_hit++;
                if (tlb_len < 16) // TLB NOT FULL
                {
                    append(&tlb, &tlb_tail, current_address);
                }
                else // TLB FULL
                {
                    FIFO_replacement(&tlb, &tlb_tail, current_address, &fifo_index_tlb);
                }
                *tlb_position = check_list(tlb, current_address->page_number);
                fprintf(output_file, "TLB: %d ", *tlb_position);
                reset_time_in_memory(&page_table, *page_table_position);
            }
            else // PAGE FAULT
            {
                page_faults++;
                if (page_table_len < 128) // PAGE TABLE NOT FULL
                {
                    tamanho_PT++;
                    append(&page_table, &page_table_tail, current_address);
                }
                else // PAGE TABLE FULL
                {
                    if (strcmp(argv[2], "lru") == 0)
                    {
                        find_next_LRU_index(&page_table, &LRU_index_page_table);
                    }
                    strcmp(argv[2], "fifo") == 0 ? FIFO_replacement(&page_table, &page_table_tail, current_address, &fifo_index_page_table) : LRU_replacement(&page_table, &page_table_tail, current_address, &LRU_index_page_table);
                }
                if (tlb_len < 16) // TLB NOT FULL
                {
                    append(&tlb, &tlb_tail, current_address);
                }
                else // TLB FULL
                {
                    FIFO_replacement(&tlb, &tlb_tail, current_address, &fifo_index_tlb);
                }

                *tlb_position = check_list(tlb, current_address->page_number);
                fprintf(output_file, "TLB: %d ", *tlb_position);
            }
        }
        reset_time_in_memory(&page_table, *page_table_position);

        if (strcmp(argv[2], "lru") == 0)
        {
            update_time_in_memory(&page_table);
        }

        *page_table_position = check_list(page_table, current_address->page_number);
        current_address->physical_address = current_address->page_offset + (*page_table_position * 256);
        read_binary_file("BACKING_STORE.bin", current_address);
        fprintf(output_file, "Physical address: %d ", current_address->physical_address);
        fprintf(output_file, "Value: %d\n", current_address->value);
    }
    fclose(ptr_file);

    page_fault_rate = ((float)page_faults / lines);
    tlb_hit_rate = ((float)tlb_hit / lines);

    fprintf(output_file, "Number of Translated Addresses = %d", lines);
    fprintf(output_file, "\nPage Faults = %d", page_faults);
    fprintf(output_file, "\nPage Fault Rate = %.3f", page_fault_rate);
    fprintf(output_file, "\nTLB Hits = %d", tlb_hit);
    fprintf(output_file, "\nTLB Hit Rate = %.3f", tlb_hit_rate);
    fclose(output_file);
    free(line);
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

void read_address(FILE *file, memory_address **address)
{
    *address = (memory_address *)malloc(sizeof(memory_address));
    fscanf(file, "%d", &((*address)->virtual_address));
    (*address)->virtual_address = ((*address)->virtual_address) & 0xFFFF;
    (*address)->page_number = ((*address)->virtual_address) >> 8;
    (*address)->page_offset = ((*address)->virtual_address) & 0xFF;
}

int check_list(memory_address *list, int page_number)
{
    memory_address *current = list;
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

void read_binary_file(const char *filename, memory_address *current_address)
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

void append(memory_address **list, memory_address **list_tail, memory_address *current)
{
    memory_address *new_node = (memory_address *)malloc(sizeof(memory_address));
    if (new_node == NULL)
    {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    memcpy(new_node, current, sizeof(memory_address));
    new_node->time_in_memory = 0;
    new_node->next = NULL;
    new_node->prev = NULL;

    if (*list == NULL)
    {
        *list = new_node;
        *list_tail = new_node;
    }
    else
    {
        (*list_tail)->next = new_node;
        new_node->prev = *list_tail;
        *list_tail = new_node;
    }
}

int len(memory_address *head)
{
    int count = 0;
    while (head != NULL)
    {
        count++;
        head = head->next;
    }
    return count;
}

void FIFO_replacement(memory_address **list, memory_address **list_tail, memory_address *current, int *fifo_index)
{
    memory_address *temp = *list;
    for (int i = 0; i < *fifo_index && temp != NULL; i++)
    {
        temp = temp->next;
    }

    if (temp != NULL)
    {
        temp->virtual_address = current->virtual_address;
        temp->page_number = current->page_number;
        temp->page_offset = current->page_offset;
        temp->physical_address = current->physical_address;
        temp->value = current->value;
        temp->time_in_memory = 0;
    }

    *fifo_index = (*fifo_index + 1) % len(*list);
}

void LRU_replacement(memory_address **list, memory_address **list_tail, memory_address *current, int *LRU_index)
{
    memory_address *temp = *list;
    for (int y = 0; y < *LRU_index && temp != NULL; y++)
    {
        temp = temp->next;
    }

    if (temp != NULL)
    {
        temp->virtual_address = current->virtual_address;
        temp->page_number = current->page_number;
        temp->page_offset = current->page_offset;
        temp->physical_address = current->physical_address;
        temp->value = current->value;
        temp->time_in_memory = 0;
    }
}

void reset_time_in_memory(memory_address **list, int index)
{
    if (index == -1)
    {
        return;
    }
    memory_address *temp = *list;
    for (int i = 0; i < index && temp != NULL; i++)
    {
        temp = temp->next;
    }

    if (temp != NULL)
    {
        temp->time_in_memory = 0;
    }
}

void update_time_in_memory(memory_address **list)
{
    memory_address *temp = *list;
    while (temp != NULL)
    {
        temp->time_in_memory += 1;
        temp = temp->next;
    }
}

void find_next_LRU_index(memory_address **list, int *LRU_index)
{
    int max_time_index = 0;
    int max_time = -1;
    int current_index = 0;
    memory_address *temp = *list;

    while (temp != NULL)
    {
        if (temp->time_in_memory > max_time)
        {
            max_time = temp->time_in_memory;
            max_time_index = current_index;
        }
        temp = temp->next;
        current_index++;
    }
    *LRU_index = max_time_index;
}
