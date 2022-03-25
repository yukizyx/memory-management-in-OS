#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

// page size = 256 byte = 2 ** 8
// logical space = 16 bits, physical space = 15 bits
// page number bits = offset bits = 8 bits
#define OFFSET_MASK 255
#define OFFSET_BITS 8

#define PAGES 256  // logical memory space / page size
#define FRAMES 128 // physical memory space / page size

#define PAGE_SIZE 256
#define BUFFER_SIZE 10 // Max number read
#define TLB_SIZE 16
#define MEMORY_SIZE (FRAMES * PAGE_SIZE)

// define TLB
struct TLBenrty
{
    int page, frame;
};

struct TLBenrty TLB[TLB_SIZE];

// simulation for a circular array.
int frame_counter = -1;
int tlb_counter = -1;

// counters
int page_fault_counter = 0;
int address_counter = 0;
int tlb_hit_counter = 0;

// initialize page table
int page_table[PAGES];

// backing store pointer
char *mmapfptr;

// native byte representation of memory
char memory[MEMORY_SIZE];


// TLB functions
int TLB_search(int pg_number)
{
    for (int i = 0; i < TLB_SIZE; i++)
    {
        if (TLB[i].page == pg_number)
        {
            ++tlb_hit_counter;
            return TLB[i].frame;
        }
    }
    return -2; // -2 for not hit and -1 for page fault
}

void TLB_Add(int pg_number, int frame_number)
{
    ++tlb_counter;
    int tlb_index = tlb_counter % TLB_SIZE;
    TLB[tlb_index].frame = frame_number;
    TLB[tlb_index].page = pg_number;
}


void TLB_Update(int pg_number, int frame_number)
{
    for (int i = 0; i < TLB_SIZE; i++)
    {
        // search for the TLB with pg_number
        if (TLB[i].frame == frame_number)
		{
            TLB[i].page = pg_number;
        }
    }
}

// search for avaliable frame with FIFO policy
int select_frame()
{
    ++page_fault_counter;
    ++frame_counter;
    if (frame_counter < FRAMES)
    {
        return frame_counter;
    }
    else
    {
        // find the original page the linked to the frame
        int frame_index = frame_counter % FRAMES;
        for (int i = 0; i < PAGES; i++)
		{
            if (page_table[i] == frame_index)
			{
                page_table[i] = -1;//invalidate it
                break;
            }
        }
        return frame_index;
    }
}

void page_fault_handler(int pg_number)
{
    int frame = page_table[pg_number];
    TLB_Update(pg_number, frame);
    //copy memory from mmap file to memory array.
    memcpy(memory + frame * PAGE_SIZE, mmapfptr + pg_number * PAGE_SIZE, PAGE_SIZE);
}

int main(int argc, const char *argv[])
{
    // open the address
    FILE *fptr = fopen("addresses.txt", "r");
    // open backing store
    int mmapfile_fd = open("BACKING_STORE.bin", O_RDONLY);

    // backing pointer
    mmapfptr = mmap(0, PAGES * PAGE_SIZE, PROT_READ, MAP_PRIVATE, mmapfile_fd, 0);

    // create buffer to load each virtual address
    char buff[BUFFER_SIZE];

    // init page table
    for (int i = 0; i < PAGES; i++)
    {
        page_table[i] = -1;
    }

    // init variables
    char value;
    int virtual_address;
    int page_number;
    int offset;
    int physical_address;
    int frame_number;
    int tlb_result;

    // initialize TLB
    for (int i = 0; i < TLB_SIZE; i++)
    {
        TLB[i].frame = -1;
        TLB[i].page = -1;
    }

    // read each line of virtual address and do operations
    while (fgets(buff, BUFFER_SIZE, fptr) != NULL)
    {
        ++address_counter;

        // calculate virtual address, page number and offset
        virtual_address = atoi(buff);
        page_number = virtual_address >> OFFSET_BITS;
        offset = virtual_address & OFFSET_MASK;

        //search in tlb
        tlb_result = TLB_search(page_number);
        if (tlb_result == -2)
		{// tlb not hit
            if (page_table[page_number] == -1)
			{//page fault
                page_table[page_number] = select_frame();
                page_fault_handler(page_number);
            }
            frame_number = page_table[page_number];
            TLB_Add(page_number, frame_number);
        }
        else
        { // tlb hit and page valid
            frame_number = tlb_result;
        }

        // calculate physical address
        physical_address = (frame_number << OFFSET_BITS) | offset;

        // get value
        value = memory[physical_address];

        // output result
        printf("Virtual address: %d Physical address = %d Value=%d \n", virtual_address, physical_address, value);
    }

    // close all file and unmap file
    close(mmapfile_fd);
    munmap(mmapfptr, PAGES * PAGE_SIZE);
    fclose(fptr);

    printf("Total addresses = %d \n", address_counter);
    printf("Page_faults = %d \n", page_fault_counter);
    printf("TLB Hits = %d \n", tlb_hit_counter);

    return 0;
}