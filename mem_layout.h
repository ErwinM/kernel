
#ifndef INCLUDE_MEM_LAYOUT_H
#define INCLUDE_MEM_LAYOUT_H

// Not trying to be efficient with memory, trying to keep it as
// simple as possible

#define KSTACK_BOTTOM 0xBFE000			// Kernel stack always in this spot
#define PGDIR_CPROC 0xA000000				// Pagedir of current process

#define FIRSTMB 0xC00000         		// First MB physical is mapped to here
#define KCRITICAL 0xd00000					// Kernel code and critical data structures

#define KHEAP_START       0xE00000
#define KHEAP_END  				0xF00000

#define PHYSTOP 0x1000000           // Top physical memory

#endif
