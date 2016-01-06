/* kheap.c
 * This implements dynamic memory allocation through a heap mechanism. There
 * will be 2 heaps: one for the kernel area and one for the user area. The heap
 * will be implemented as a single linked ordered list
 *
 * Initially the following simplifications will be used:
 * - both heaps will be fixed in size (no expand, contract);
 * - ...
