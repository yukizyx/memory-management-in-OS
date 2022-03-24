#include <stdio.h>
#include <unistd.h>

// page size = 256 byte = 2 ** 8
// logical space = 16 bits, physical space = 15 bits
// page number bits = offset bits = 8 bits
#define OFFSET_MASK 255
#define PAGES 256 // logical memory space / page size
#define OFFSET_BITS 8
#define PAGE_SIZE 256
#define BUFFER_SIZE 10 // ??
#define TLB_SIZE 16

int main(int argc, const char *argv[])
{
    // open the file
    FILE *fptr = fopen("addresses.txt", "r");

    // create buffer to load each virtual address
    char buff[BUFFER_SIZE];

    int i;

    int virtual_address;
    int page_number;
    int offset;

    int physical_address;
    int frame_number;

    int page_faults;

    // initialize page table
    int page_table[PAGES];
    for (i = 0; i < PAGES; i++)
    {
        page_table[i] = -1;
    }

    // initialize TLB

    while (fgets(buff, BUFFER_SIZE, fptr) != NULL)
    {
        // calculate virtual address, page number and offset
        virtual_address = atoi(buff);

        page_number = virtual_address >> OFFSET_BITS;
        offset = virtual_address & OFFSET_MASK;

        // search in page table
        int flag = 0;
        for (i = 0; i < PAGES; i++)
        {
            if (page_table[i] == page_number)
            {
                frame_number = i;
                page_faults += 1;
                break;
            }
            if (page_table[i] == -1)
            {
                flag = 1;
                break;
            }
        }

        if (flag == 1)
        {
            page_table[i] = page_number;
            frame_number = i;
        }

        // calculate physical address
        physical_address = (frame_number << OFFSET_BITS) | offset;

        // output result
        printf("Virtual address: %d Physical address = %d Value=0 \n", virtual_address, physical_address);
    }
    fclose(fptr);

    printf("page table = {");
    for (int i = 0; i < PAGES; i++)
    {
        printf(" %d ", page_table[i]);
    }
    printf("}\n");

    printf("Page_faults = %d ", page_faults);

    return 0;
}