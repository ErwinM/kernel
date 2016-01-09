/** kheap.c
 * This implements dynamic memory allocation through a heap mechanism. There
 * will be 2 heaps: one for the kernel area and one for the user area. The heap
 * will be implemented as a single linked ordered list
 *
 * Initially the following simplifications will be used:
 * - both heaps will be fixed in size (no expand, contract);
 * - no swapping to secondary memory
 * - kernel memory is identity mapped at paging_init
 */

#include "common.h"
#include "kheap.h"
#include "ordered_list.h"
#include "paging.h"

heap_t *kheap;

uint8_t size_less_than(heap_header_t* a, heap_header_t* b)
{
	return ((heap_header_t*)a->size < (heap_header_t*)b->size)?1:0;
}

heap_t *create_heap(uint32_t start, uint32_t end, uint8_t kernel_mode, uint8_t read_only)
{
	// first we need some memory to keep the heap meta-data
	// this is a permanent kernel data structure so space is allocated
	// in the critical kernel data region
	heap_t *heap = (heap_t *)kmalloc_a(sizeof(heap_t));

	// we place the (dynamic in size) index in the heap area itself
	heap->index = place_ordered_list((void*)start, HEAP_INDEX_SIZE, &size_less_than);

	// we adjust the start to account for the index
	start += sizeof(type_t)*HEAP_INDEX_SIZE;

	// make sure that the new start is page aligned
	if (start & 0xFFFFF000 != 0)
	{
		start &= 0xFFFFF000;
		start += 0x1000;
	}

	// Fill in the heap meta-data
	heap->start_address = start;
	heap->end_address = end;
	heap->kernel_mode = kernel_mode;
	heap->read_only = read_only;

	// Initialise the index which at this point consists of a single hole
	heap_header_t *header = (heap_header_t *)start;
	header->is_hole = 1;
	header->magic = HEAP_MAGIC;
	header->size = end - start;
	insert_ordered_list((void*)header, &heap->index);
	return heap;
}
