/** paging.c:
	* includes kernel memory allocation functions and paging code
	*/

#include "paging.h"
#include "common.h"
#include "isr.h"
#include "write.h"

extern uint32_t end;
uint32_t placement_address = (uint32_t)&end;

// The kernel's page directory
page_directory_t *kernel_page_directory=0;

// The current page directory;
page_directory_t *current_page_directory=0;

uint32_t *frames;
uint32_t total_frames;

uint32_t kmalloc(uint32_t size, int align, uint32_t *phys)
{
	fb_write("\nPlacement pointer: ");
	fb_write_hex(placement_address);
	if (align == 1 && (placement_address & 0xFFFFF000)) // if address is not 4k-aligned
	{
		// Align it
		placement_address &= 0xFFFFF000;
		placement_address += 0x1000;
	}
if (phys)
{
	*phys = placement_address;
}
	uint32_t placed_at = placement_address;
	fb_write(", Requested size: ");
	fb_write_dec(size);
	fb_write(", Placed at: ");
	fb_write_hex(placed_at);
	fb_write("\n");
	placement_address += size;
	return placed_at;
}

uint32_t kmalloc_a(uint32_t size)
{
	return kmalloc(size, 1, 0);
}

uint32_t kmalloc_ap(uint32_t size, uint32_t *phys)
{
	return kmalloc(size, 1, phys);
}

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

// Static function to set a bit in the frames bitset
static void set_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr/0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
static void clear_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr/0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
static uint32_t test_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr/0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}

// Static function to find the first free frame.
static uint32_t first_free_frame()
{
    uint32_t i, j;
    for (i = 0; i < INDEX_FROM_BIT(total_frames); i++)
    {
        if (frames[i] != 0xFFFFFFFF) // nothing free, exit early.
        {
            // at least one bit is free here.
            for (j = 0; j < 32; j++)
            {
                uint32_t toTest = 0x1 << j;
                if ( !(frames[i]&toTest) )
                {
										// return the index of the free frame
										return i*4*8+j;
                }
            }
        }
    }
}

void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
	if ( page->frame != 0 )
	{
		return; // Frame already allocated
	}
	else
	{
		uint32_t idx = first_free_frame(); // idx is now the index of the first free frame.
   	if (idx == (uint32_t)-1)
   	{
       // PANIC is just a macro that prints a message to the screen then hits an infinite loop.
       PANIC("No free frames!");
   	}
   	set_frame(idx*0x1000); // this frame is now ours!
   	page->present = 1; // Mark it as present.
   	page->rw = (is_writeable)?1:0; // Should the page be writeable?
   	page->user = (is_kernel)?0:1; // Should the page be user-mode?
   	page->frame = idx;
	}

}

page_t *get_page(uint32_t address, int make, page_directory_t *dir)
{
   // Turn the address into an index.
   uint32_t frame_number = address / 0x1000;
   // Find the page table containing this address.
   uint32_t table_idx = frame_number / 1024;
   if (dir->tables[table_idx]) // If this table is already assigned
   {
       return &dir->tables[table_idx]->pages[frame_number%1024];
   }
   else if(make)
   {
       uint32_t tmp;
       dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
       memset(dir->tables[table_idx], 0, 0x1000);
       dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
       return &dir->tables[table_idx]->pages[frame_number%1024];
   }
   else
   {
       return 0;
   }
}

void initialise_paging()
{
	// Size of physical memory; set at 16MB
	uint32_t size_of_memory = 0x1000000;

	// Setup the space for the bitmap that tracks frame usage
	fb_write("Building frame bitmap...");
	total_frames = size_of_memory / 0x1000;
	frames = (uint32_t*)kmalloc_a(INDEX_FROM_BIT(total_frames));
	memset(frames, 0, INDEX_FROM_BIT(total_frames));

	// Setup the kernal page_directory
	fb_write("Building kernel_page_directory...");
	kernel_page_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
	memset(kernel_page_directory, 0, sizeof(page_directory_t));


	// We need to identity map (virtual == physical) the kernel memory used
	// so far or the kernel will be lost after turning on paging; luckily
	// placement_address points to the end of our used memory so far.
	fb_write("Identity mapping kernel memory...");
	int i = 0;
	while (i < placement_address)
	{
		alloc_frame(get_page(i, 1, kernel_page_directory), 0,0);
		i += 0x1000;
	}

	// Before we enable paging, we must register our page fault handler.
  install_irq_handler(14, page_fault);

  // Now, enable paging!
	fb_write("Enabling paging...");
  switch_page_directory(kernel_page_directory);
}

void switch_page_directory(page_directory_t *dir)
{
    current_page_directory = dir;
    asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void page_fault(regs_t regs)
{
   // A page fault has occurred.
   // The faulting address is stored in the CR2 register.
   uint32_t faulting_address;
   asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

   // The error code gives us details of what happened.
   int present   = !(regs.err_code & 0x1); // Page not present
   int rw = regs.err_code & 0x2;           // Write operation?
   int us = regs.err_code & 0x4;           // Processor was in user-mode?
   int reserved = regs.err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
   int id = regs.err_code & 0x10;          // Caused by an instruction fetch?

   // Output an error message.
   fb_write("Page fault! ( ");
   if (present) {fb_write("present ");}
   if (rw) {fb_write("read-only ");}
   if (us) {fb_write("user-mode ");}
   if (reserved) {fb_write("reserved ");}
   fb_write(" at ");
   fb_write_hex(faulting_address);
   fb_write("\n");
   PANIC("Page fault");
}
