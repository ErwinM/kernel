
#ifndef INCLUDE_MEM_LAYOUT_H
#define INCLUDE_MEM_LAYOUT_H

// Not trying to be efficient with memory, trying to keep it as
// simple as possible


#define FIRSTMB 0x400000         		// First MB physical is mapped to here
#define KCRITICAL 0x500000					// Kernel code and critical data structures

#define KHEAP_START	0x600000
#define KHEAP_END	0x800000

#define PHYSTOP 0x1000000           // Top physical memory

#define NSEGS	6

#endif
