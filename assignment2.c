#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// page size = 256 byte = 2 ** 8
// logical space = 16 bits, physical space = 15 bits
// page number bits = offset bits = 8 bits

// address calculation
#define OFFSET_MASK 255
#define OFFSET_BITS 8

// total = 256 pages, 128 frames
#define PAGE_NUM 256 // logical memory space / page size
#define FRAME_NUM 128

#define PAGE_SIZE 256
#define FRAME_SIZE 256
#define BUFFER_SIZE 10 // ??
#define TLB_SIZE 16
#define MEMORY_SIZE 32768

int i;
int page_faults = 0;
int current_frame = 0;
int page_table[PAGE_NUM];
int page_in_memo[PAGE_NUM];

int select_frame()
{
    int selected;
    if (current_frame < FRAME_NUM)
    {
        selected = current_frame;
        current_frame++;
        return selected;
    }
    else
    {
        selected = (current_frame) % FRAME_NUM;
        for (i = 0; i < PAGE_NUM; ++i)
        {
            if (page_table[i] == selected && page_in_memo[i] == 1)
            {
                page_in_memo[i] = 0; // invalidate it
                break;
            }
        }
        current_frame++;
        return selected;
    }
}

int main(int argc, const char *argv[])
{
    // open the file
    FILE *fptr = fopen("addresses.txt", "r");
    FILE *storage = fopen("BACKING_STORE.bin", "rb");

    // create buffer to load each virtual address
    char buff[BUFFER_SIZE];
    char main_memory[MEMORY_SIZE];

    int virtual_address;
    int page_number;
    int offset;

    int physical_address;
    int frame_number;

    // initialize page table

    for (i = 0; i < PAGE_NUM; i++)
    {
        page_table[i] = -1;
    }

    // initialize pages in main memory

    for (i = 0; i < PAGE_NUM; i++)
    {
        page_in_memo[i] = 0;
    }

    // initialize TLB

    while (fgets(buff, BUFFER_SIZE, fptr) != NULL)
    {
        // calculate virtual address, page number and offset
        virtual_address = atoi(buff);

        page_number = virtual_address >> OFFSET_BITS;
        offset = virtual_address & OFFSET_MASK;

        // check whether page in main memory
        if (page_in_memo[page_number] == 1)
        {
            // page in main memory
            frame_number = page_table[page_number];
        }
        else
        {
            // page not in main memory --> page fault
            page_faults += 1;

            page_table[page_number] = select_frame();

            // fseek(storage, page_number * PAGE_SIZE, SEEK_SET);
            // fread(main_memory + page_table[page_number] * PAGE_SIZE, sizeof(int8_t), PAGE_SIZE, storage);

            page_in_memo[page_number] = 1;

            frame_number = page_table[page_number];
        }

        // calculate physical address
        physical_address = (frame_number << OFFSET_BITS) | offset;

        // output result
        // printf("Virtual address: %d Physical address = %d Value=0 \n", virtual_address, physical_address);
        printf("Virtual address: %d Physical address = %d Value=0, %d, %d \n", virtual_address, physical_address, page_number, frame_number);
    }

    // close the file
    fclose(fptr);
    fclose(storage);

    printf("page table = {");
    for (int i = 0; i < PAGE_NUM; i++)
    {
        printf(" %d ", page_table[i]);
    }
    printf("}\n");

    printf("Page_faults = %d ", page_faults);

    return 0;
}