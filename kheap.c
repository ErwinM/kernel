/** kheap.c
 * This implements dynamic memory allocation through a heap mechanism. There
 * will be 2 heaps: one for the kernel area and one for the user area. The heap
 * will be implemented as a single linked ordered list
 *
 * Initially the following simplifications will be used:
 * - both heaps will be fixed in size (no expand, contract);
 * - no swapping to secondary memory
 * - kernel memory is identity mapped at paging_init
 * - first fit algorithm
 */

#include "common.h"
#include "kheap.h"
#include "ordered_list.h"
#include "paging.h"

heap_t *kheap;

uint8_t address_less_than(heap_header_t a, heap_header_t b)
{
	return (&a < &b)?1:0;
}

heap_t *create_heap(uint32_t start, uint32_t end, uint8_t kernel_mode, uint8_t read_only)
{
	// first we need some memory to keep the heap meta-data
	// this is a permanent kernel data structure so space is allocated
	// in the critical kernel data region
	heap_t *heap = (heap_t *)kmalloc_a(sizeof(heap_t));

	// we place the (dynamic in size) index in the heap area itself
fb_write("Allocating for index now...");
	heap->index = place_ordered_list((void*)start, HEAP_INDEX_SIZE, &address_less_than);

	// we adjust the start to account for the index
	start += sizeof(uint32_t)*HEAP_INDEX_SIZE;

	// make sure that the new start is page aligned
	if (start & ~0xFFFFF000)
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
	fb_printf("Location of 1st header: %h", header);
	fb_printf("Index located at: %h", heap->index);;
	insert_ordered_list((void*)header, &heap->index);
	uint32_t *tmp = lookup_ordered_list(0, &heap->index);
	fb_printf("Lookup returned: %h", tmp);

	return heap;
}

int32_t find_first_hole(uint32_t size, uint8_t page_align, heap_t *heap)
{
		int32_t iterator = 0;
		while (iterator < heap->index.size)
		{
			heap_header_t *header = (heap_header_t *)lookup_ordered_list(iterator, &heap->index);
			uint32_t loc = (uint32_t)header;
			int32_t offset = 0;
			int32_t aligned_hole_start = 0;
			int32_t remaining_hole_size = 0;
			if (page_align && (loc+sizeof(heap_header_t) & ~0xfffff000))
			// page aligned memory requested, but hole is not page aligned
			{
				// make sure that hole is still big enough after page align
				offset = 0x1000 - (loc + sizeof(heap_header_t))%0x1000;
				int32_t aligned_hole_size = (int32_t)header->size - offset;
				// make sure that the area before the header is feasible as a hole
				// otherwise this memory would be in limbo
				aligned_hole_start = loc & 0xFFFF000;
				aligned_hole_start += 0x1000;
				remaining_hole_size = aligned_hole_start - loc - sizeof(heap_header_t) - sizeof(heap_footer_t);
				if (aligned_hole_size >= size && remaining_hole_size > 0)
				{
					break;
				}
			} else {
				if (header->size > size)
				{
					break;
				}
			}
			iterator++;
		}
		// why did the loop exit?
		if (iterator == heap->index.size)
		{
			// no hole found of appropriate size!
			return -1;
		} else {
			fb_printf("Returning: %d", iterator);
			return iterator;
		}
}

void *alloc( uint32_t size, uint8_t page_align, heap_t *heap)
{
	// lets ignore page alignment intially....
	uint32_t gross_size = size + sizeof(heap_header_t) + sizeof(heap_footer_t);

	// find hole
	uint32_t idx = find_first_hole(gross_size, page_align, heap);
	if (idx == -1 )
	{
		PANIC("Out of heap memory!");
	}
	heap_header_t *original_hole_header = (heap_header_t *)lookup_ordered_list(idx, &heap->index);
	fb_printf("Found header at: %h", original_hole_header);
	uint32_t block_pos = original_hole_header;
	uint32_t original_hole_size = original_hole_header->size;
	// do we split the hole?
	if ((original_hole_header->size-gross_size) < (sizeof(heap_header_t) + sizeof(heap_footer_t)));
	{
		// not worth splitting, adjust requested size to hole size
		gross_size = original_hole_header->size;
		size = original_hole_header - sizeof(heap_header_t) + sizeof(heap_footer_t);
	}
	// If we need to page-align the data, do it now and make a new hole in front of our block.
	if (page_align && (block_pos+sizeof(heap_header_t)) & ~0xFFFFF000)
	{
		// we need to create a new hole from the area before our aligned address
		// we can repurpose the hole we found (since it is getting allocated)
		uint32_t aligned_hole_header = block_pos & 0xFFFFF000;
		aligned_hole_header += 0x1000;
		aligned_hole_header -= sizeof(heap_header_t);
		heap_header_t *new_hole_header = (heap_header_t *)aligned_hole_header;
		new_hole_header->size = aligned_hole_header - block_pos - sizeof(heap_header_t) - sizeof(heap_footer_t);
		new_hole_header->is_hole = 1;
		new_hole_header->magic = HEAP_MAGIC;
		heap_footer_t *new_hole_footer = (heap_footer_t *)(aligned_hole_header - sizeof(heap_footer_t));
		new_hole_footer->header_ptr = new_hole_header;
		new_hole_footer->magic = HEAP_MAGIC;
		block_pos = aligned_hole_header;
		original_hole_size -= (new_hole_header->size + sizeof(heap_header_t) + sizeof(heap_footer_t));
	} else {
		remove_ordered_list(idx, &heap->index);
	}
	// now we write the correct headers for the allocated block
	heap_header_t *block_header = (heap_header_t *)block_pos;
	block_header->size = gross_size;
	block_header->magic = HEAP_MAGIC;
	block_header->is_hole = 0;
	// ... and footer
	heap_footer_t *block_footer = (heap_footer_t *)(block_pos + size + sizeof(heap_header_t));
	block_footer->magic = HEAP_MAGIC;
	block_footer->header_ptr = block_pos;

	if ((original_hole_size - gross_size) > 0)
	{
		heap_header_t *post_header = (heap_header_t *)(block_pos + gross_size);
		post_header->size = original_hole_size - gross_size;
		post_header->magic = HEAP_MAGIC;
		post_header->is_hole = 1;
		heap_footer_t *post_footer_pos = (heap_footer_t *)(original_hole_header + original_hole_size - sizeof(heap_footer_t));
		if ((uint32_t)post_footer_pos < heap->end_address)
		{
			post_footer_pos->magic = HEAP_MAGIC;
			post_footer_pos->header_ptr = post_header;
		}
		insert_ordered_list((void *)post_header, &heap->index);
	}
	return (void *)(block_header + sizeof(heap_header_t));
}
