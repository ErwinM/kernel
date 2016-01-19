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
	//fb_printf("Location of 1st header: %h", header);
	//fb_printf("Index located at: %h", heap->index);;
	insert_ordered_list((void*)header, &heap->index);
	//uint32_t *tmp = lookup_ordered_list(0, &heap->index);
	//fb_printf("heap size: %h", header->size);

	return heap;
}

int32_t find_first_hole(uint32_t size, uint8_t page_align, heap_t *heap)
{
		int32_t iterator = 0;
		while (iterator < heap->index.size)
		{
			uint32_t lookup = lookup_ordered_list(iterator, &heap->index);
			fb_printf("Lookup ordered list returned: %h", lookup);
			heap_header_t *header = (heap_header_t *)lookup_ordered_list(iterator, &heap->index);
			fb_printf("Evaluating header at: %h", header);
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
					//fb_printf("Iteration: %d\n", iterator);
					//fb_printf("AHS: %h\n", header->size);
					//fb_printf("size: %h\n", size);
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
			//fb_printf("Returning: %d", iterator);
			return iterator;
		}
}

void *alloc( uint32_t size, uint8_t page_align, heap_t *heap)
{
	// lets ignore page alignment intially....
	uint32_t gross_size = size + sizeof(heap_header_t) + sizeof(heap_footer_t);
	//fb_printf("gross_size: %d", gross_size);
	// find hole
	uint32_t idx = find_first_hole(gross_size, page_align, heap);
	if (idx == -1 )
	{
		PANIC("Out of heap memory!");
	}
	heap_header_t *original_hole_header = (heap_header_t *)lookup_ordered_list(idx, &heap->index);
	//fb_printf("Found header at: %h", original_hole_header);
	uint32_t block_pos = original_hole_header;
	uint32_t original_hole_size = original_hole_header->size;
	//fb_printf("original_hole_size: %d", original_hole_size);
	// do we split the hole?
	if ((original_hole_size-gross_size) < (sizeof(heap_header_t) + sizeof(heap_footer_t)))
	{
		//fb_write("HIT!");
		// not worth splitting, adjust requested size to hole size
		gross_size = original_hole_header->size;
		size = original_hole_header->size - sizeof(heap_header_t) + sizeof(heap_footer_t);
	}
	// If we need to page-align the data, do it now and make a new hole in front of our block.
	if (page_align && (block_pos+sizeof(heap_header_t)) & ~0xFFFFF000)
	{
		// we need to create a new hole from the area before our aligned address
		// we can repurpose the hole we found (since it is getting allocated)
		uint32_t aligned_block_header = block_pos & 0xFFFFF000;
		aligned_block_header += 0x1000;
		aligned_block_header -= sizeof(heap_header_t);
		heap_header_t *pre_hole_header = (heap_header_t *)block_pos;
		pre_hole_header->size = aligned_block_header - block_pos;
		fb_printf("PRE HOLE SIZE: %h", pre_hole_header->size);
		pre_hole_header->is_hole = 1;
		pre_hole_header->magic = HEAP_MAGIC;
		heap_footer_t *pre_hole_footer = (heap_footer_t *)((uint32_t)aligned_block_header - sizeof(heap_footer_t));
		pre_hole_footer->header_ptr = pre_hole_header;
		pre_hole_footer->magic = HEAP_MAGIC;
		block_pos = aligned_block_header;
		original_hole_size -= (uint32_t)pre_hole_header->size;
	} else {
		remove_ordered_list(idx, &heap->index);
	}
	// now we write the correct headers for the allocated block
	heap_header_t *block_header = (heap_header_t *)block_pos;
	block_header->is_hole = 0;
	block_header->size = gross_size;
	block_header->magic = HEAP_MAGIC;
	//fb_printf("Block->size: %d", block_header->size);
	// ... and footer
	//fb_printf("block_footer->size: %h", size);
	heap_footer_t *block_footer = (heap_footer_t *)(uint32_t)(block_pos + size + sizeof(heap_header_t));


	block_footer->header_ptr = block_pos;
	block_footer->magic = HEAP_MAGIC;
	if ((original_hole_size - gross_size) > 0)
	{
		heap_header_t *post_header = (heap_header_t *)(block_pos + gross_size);
		post_header->size = original_hole_size - gross_size;
		post_header->magic = HEAP_MAGIC;
		post_header->is_hole = 1;
		heap_footer_t *post_footer_pos = (heap_footer_t *)((uint32_t)original_hole_header + original_hole_size - sizeof(heap_footer_t));
		//fb_printf("post_footer: %h", post_footer_pos);
		if ((uint32_t)post_footer_pos < heap->end_address)
		{
			post_footer_pos->magic = HEAP_MAGIC;
			post_footer_pos->header_ptr = post_header;
		}
		insert_ordered_list((void *)post_header, &heap->index);
	}
	return (void *)(uint32_t)block_header + sizeof(heap_header_t);
}
void free(void* ptr, heap_t *heap)
{
	if (ptr==0)
		return;

	heap_header_t *header = (heap_header_t*)((uint32_t) ptr - sizeof(heap_header_t));
	heap_footer_t *footer = (heap_footer_t*)((uint32_t) header + header->size - sizeof(heap_footer_t));
	ASSERT(header->magic == HEAP_MAGIC);
	ASSERT(footer->magic == HEAP_MAGIC);

	header->is_hole = 1;
	int8_t add_to_index = 1;

	// check if we should merge left
	heap_footer_t *pre_footer = (heap_footer_t*)((uint32_t)header - sizeof(heap_footer_t));
	//fb_printf("pre_footer: %h", pre_footer);

	if (pre_footer->magic == HEAP_MAGIC && pre_footer->header_ptr->is_hole == 1)
	{
		fb_write("PRE-SHOULD NOT BE HIT");
		uint32_t original_block_size = header->size; // cache size of block we are freeing
		header = pre_footer->header_ptr; // pre-header is now the new header
		header->size += original_block_size; // add the size of block to the pre-header
		footer->header_ptr = header; // make original footer point to pre-header
		pre_footer->magic = 0; // remove the pre-footer
		add_to_index = 0; // no need to add header to index, its already There
	}
	// check if we should merge right
	heap_header_t *post_header = (heap_header_t*)((uint32_t)footer + sizeof(heap_footer_t));
	//fb_printf("post_header: %h", post_header);
	ASSERT(post_header->magic == HEAP_MAGIC);

	if (post_header->magic == HEAP_MAGIC && post_header->is_hole == 1)
	{
		fb_write("POST-SHOULD NOT BE HIT");
		header->size += post_header->size;
		heap_footer_t *post_footer = (heap_footer_t*)((uint32_t)post_header + post_header->size - sizeof(heap_footer_t));
		ASSERT(post_footer->magic == HEAP_MAGIC);
		post_footer->header_ptr = header;
		footer->magic = 0;
		// we need to remove the index pointer to post_header
		uint32_t i=0;
		while (i < heap->index.size && lookup_ordered_list(i, &heap->index) != (void*)post_header)
		{
			i++;
		}
		ASSERT(i <= heap->index.size);
		remove_ordered_list(i, &heap->index);
	}
	if (add_to_index == 1)
	{
		fb_printf("Adding header: %h", header);
		//fb_printf("Heap index size: %d", heap->index.size);
		insert_ordered_list((void*)header, &heap->index);
		//fb_printf("Index after insert: %h", heap->index.list[0]);
		//fb_printf("Heap index size: %d", heap->index.size);
	}
}
