/* Allocator for the kernel heap. Memory is page mapped in advanced.
 * Noticed that without a little wiggling room (memory wise). Everything
 * becomes very hard (TM).
 */

#include "mem_layout.h"
#include "param.h"
#include "spinlock.h"
#include "mmu.h"

extern uint32_t end;

struct chunk {
  struct chunk *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct chunk *freelist;
} kmem;

extern uint32_t placement_address;

// Allocates memory starting directly at the end of loaded kernel
// This memory is permenantly allocated and cannot be freed!
uint32_t* kmalloc(uint32_t size, int align)
{
	if (align == 1 && (placement_address & ~0xFFFFF000)) // if address is not 4k-aligned
	{
		// Align it
		placement_address &= 0xFFFFF000;
		placement_address += 0x1000;
	}
	uint32_t placed_at = (uint32_t*)placement_address;
	kprintf("KMALLOC: requested: %d.", size);
	kprintf(" Placed at: %h.\n", placed_at);
	placement_address += size;
	return placed_at;
}

uint32_t* kmalloc_a(uint32_t size)
{
	return kmalloc(size, 1);
}

void initkheap()
{
	// Build the free list
	freerange(KHEAP_START, KHEAP_END);
	kprintf("Freelist: %h", kmem.freelist);
}

void freerange(uint32_t *start, uint32_t *end)
{
	if (start < KHEAP_START || end > KHEAP_END ){
		kprintf("start: %h\n", start);
		kprintf("end: %h\n", end);
		PANIC("freerange: memory not in heap!");
	}
	if ((uint32_t)end & ~0xFFFFF000)
		PANIC("freerange: end not page aligned!");

	int k;
	for( k = (end - PGSIZE) ; k >= start ; k -= PGSIZE){
		kfree(k);
	}
}

void kfree(uint32_t *address)
{
	if (address < KHEAP_START || address > KHEAP_END ){
		kprintf("address: %h\n", address);
		PANIC("kfree: memory not in heap!");
	}
	struct chunk *c;
	memset(address, 1, PGSIZE);
	c = (struct chunk*) address;
	c->next = kmem.freelist;
	kmem.freelist = c;
}

uint32_t *kalloc()
{
	struct chunk *c;
	c = kmem.freelist;
	if (c){
		kmem.freelist = c->next;
	} else {
		PANIC("kallocpage: out of heap memory!");
	}
	return c;
}
