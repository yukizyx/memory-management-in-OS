# Memory Management in OS

*A simple memory management simulator in C that supports paging including address translation, TLB implementation and page faults handling.*

- Category: C, Operating System

- Keyword: 
  - OS
  - Memory management unit
  - TLB implementation
  - Page faults handling
- Date: March, 2022


## Table of Contents

- [Memory Management in OS](#memory-management-in-os)
  * [Table of Contents](#table-of-contents)
  * [Description](#description)
    + [Setup](#setup)
    + [General Features](#general-features)
    + [MMU – Address Translation](#mmu---address-translation)
    + [Handling Page Faults](#handling-page-faults)
    + [MMU - TLB](#mmu---tlb)

## Description

This project simulates a memory management unit (MMU), which translates logical addresses to physical addresses.

### Setup

In this setup the logical address space (2^16 = 65,536 bytes) is larger than the physical address space (2^15 bytes), and the page size is 256 bytes. The maximum no. of entries in the TLB = 16.

### General Features

- Translating a logical address to physical address in this setup will involve the following: Check TLB for the page. If page not found in the TLB, check the page table if the page exists in memory. If it does not, then a page fault occurs.

- Handling page faults involves copy the page from the backing store to memory. Since logical address space is larger than physical address space, a page request might involve replacing a page in memory with the new page. The page replacement policy to be used is FIFO page replacement policy.

### MMU – Address Translation

The program will translate logical to physical addresses using a page table. 

1. To simulate a program’s memory address requests, we use the text file `addresses.txt`.

2. This file contains integer values representing logical addresses ranging from 0 − 65535 (the size of the logical address space).

3. The program will open this file using the `fopen()` library function and read each logical address from the file and compute its page number and offset using bitwise operators in C. 

4. After extracting the page number from the logical address the program will look up the TLB.
In the case of a TLB-hit (an entry corresponding to a page number exists), the frame number is obtained from the TLB. In the case of a TLB-miss (NO entry corresponding to the page number exists), look up the page table. In the latter case either the frame number is obtained from the page table, or a page fault occurs.

5. The page table can simply be an array. Since the system implements demand paging, initially all the entries of the page table can be set to -1 to indicate a page is not in memory.

### Handling Page Faults

Handling page faults involves copying the page from the backing store to a frame in memory.

1. The backing store is represented by the file `BACKING_STORE.bin`. It is a binary file of size 65,536 bytes.

2. Open this file using the `open()` library call in your program and map it to a memory region using the system call `mmap()`.

3. When page fault occurs, the program will read in a 256-byte page from this memory mapped file and copy it in an available frame in physical memory using the `memcpy()` function.

4. Since logical address space is larger than the physical address space, a page request might involve replacing a page in memory with the new page. The page replacement policy to be used is FIFO page replacement policy; that is, the oldest page in memory is replaced by the new page. 

5. The easiest way to accomplish the FIFO page replacement policy is to simulate the physical memory as a circular array.

### MMU - TLB

1. To simulate entries of TLB, the program uses a data structure called TLBentry which stores page number and frame number pair.

2. There are three functions related to the TLB:

   - A `search_TLB` function - To search the TLB for an entry corresponding to a page number.

   - A `TLB_Add` function – To add an entry to the TLB. Since TLB uses the FIFO policy, if TLB is full, any entry added to the TLB replaces the oldest entry.

   - A `TLB_Update` function – To update the TLB when a page ‘p’ is replaced in physical memory, and an entry corresponding to page ‘p’ exists in the TLB. 

3. All of the above mentioned functionality can be achieved by implementing the TLB as a circular array.
