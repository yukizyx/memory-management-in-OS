#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h> 
#include <unistd.h>
#include <fcntl.h>

// page size = 256 byte = 2 ** 8
// logical space = 16 bits, physical space = 15 bits
// page number bits = offset bits = 8 bits
#define OFFSET_MASK 255
#define PAGES 256 // logical memory space / page size
#define OFFSET_BITS 8
#define PAGE_SIZE 256
#define FRAMES 128 // physical memory space / page size
#define BUFFER_SIZE 10 // Max number read
#define TLB_SIZE 16
#define MEMORY_SIZE (FRAMES * PAGE_SIZE)
// simulation for a circular array.
int frame_counter = -1;
//counters
int page_fault_counter = 0;
int address_counter = 0;
// initialize page table
int page_table[PAGES];
//backing store pointer
char *mmapfptr ;
//native byte representation of memory
char memory[MEMORY_SIZE];

int select_frame(){
    ++ page_fault_counter;
    ++ frame_counter;
    // printf("frame count: %d ", frame_counter);
    if(frame_counter < FRAMES){
        // printf("\n");
        return frame_counter;
    }else{
        //find the original page the linked to the frame
        int frame_index = frame_counter % FRAMES;
        // printf("frame index: %d ", frame_index);
        for (int i = 0; i < PAGES; i++){
            if (page_table[i] == frame_index){
                page_table[i] = -1;//invalidate it
                break;
            }
        }
        // printf("\n");
        return frame_index;
    }
}

void page_fault_handler(int pg_number){
    int frame = page_table[pg_number];
    memcpy(memory + frame * PAGE_SIZE, mmapfptr + pg_number * PAGE_SIZE, PAGE_SIZE);
}

int main(int argc, const char *argv[])
{
    // open the address
    FILE *fptr = fopen("addresses.txt", "r");
    //open backing store
    int mmapfile_fd = open("BACKING_STORE.bin", O_RDONLY);
    //backing pointer
    mmapfptr = mmap(0, PAGES * PAGE_SIZE, PROT_READ, MAP_PRIVATE, mmapfile_fd, 0);

    // create buffer to load each virtual address
    char buff[BUFFER_SIZE];

    //init page table
    for (int i = 0; i < PAGES; i++)
    {
        page_table[i] = -1;
    }
    // init variables
    int value;
    int virtual_address;
    int page_number;
    int offset;
    int physical_address;
    int frame_number;
    // initialize TLB

    while (fgets(buff, BUFFER_SIZE, fptr) != NULL)
    {
        ++ address_counter;
        
        // calculate virtual address, page number and offset
        virtual_address = atoi(buff);

        page_number = virtual_address >> OFFSET_BITS;
        // page_number = GET_PAGE_NUMBER(virtual_address);
        offset = virtual_address & OFFSET_MASK;

        // search in page table
        if (page_table[page_number] == -1){
            page_table[page_number] = select_frame();
            page_fault_handler(page_number);
        }
        frame_number = page_table[page_number];
        

        // calculate physical address
        physical_address = (frame_number << OFFSET_BITS) | offset;
        //get value
        
        value = memory[physical_address];
        // output result
        printf("Virtual address: %d Physical address = %d Value=%d \n", virtual_address, physical_address, value);
    }
    //close all file and unmap file
    close(mmapfile_fd);
    munmap(mmapfptr, MEMORY_SIZE);
    fclose(fptr);

    // printf("page table = {");
    // for (int i = 0; i < PAGES; i++)
    // {
    //     printf("%d, ", page_table[i]);
    // }
    // printf("}\n");
    printf("total addresses = %d \n", address_counter);
    printf("Page_faults = %d \n", page_fault_counter);

    return 0;
}