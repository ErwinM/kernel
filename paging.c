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

// last page in vmem is mapped to the page dir, so we can edit pdes this way once paging is enabled
uint32_t *kernel_page_dir = (uint32_t*) 0xFFFFF000;
uint32_t *current_page_dir;

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
    for (i = 0; i < INDEX_FROM_BIT(total_frames); i++) // loop over every bitmask byte
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
										// e.g. the nth frame out of 4096 frames (16mb) is free
										return i*4*8+j;
                }
            }
        }
    }
}

pageinfo mm_virtaddrtopageindex(uint32_t virtaddr){
    pageinfo pginf;

    //align address to 4k (highest 20-bits of address)
    virtaddr &= ~0xFFF;
    pginf.pagetable = virtaddr / 0x400000; // each page table covers 0x400000 bytes in memory
    pginf.page = (virtaddr % 0x400000) / 0x1000; //0x1000 = page size
    return pginf;
}

uint32_t mm_allocphyspage()
{
	uint32_t idx = first_free_frame(); // idx is now the index of the first free frame
	uint32_t phys_address = (idx * 0x1000);
	set_frame(phys_address); // this frame is now ours!
	memset(phys_address, 0, 4096); // clear the frame
	return (phys_address);
}

uint32_t mm_mappage(uint32_t phys_address, uint32_t virt_address)
{
  pageinfo pginf = mm_virtaddrtopageindex(virt_address); // get the PDE and PTE indexes for the addr

  if(kernel_page_dir[pginf.pagetable] & 1)
	{
      // page table exists.
      uint32_t *page_table = (uint32_t *) (0xFFC00000 + (pginf.pagetable * 0x1000)); // virt addr of page table
      if(!page_table[pginf.page] & 1)
			{
          // page isn't mapped
          page_table[pginf.page] = phys_address | 3;
      }
			else
			{
          // page is already mapped
          PANIC("PAGE ALREADY MAPPED");
      }
  	}
		else
		{
      // page table doesn't exist, so alloc a page and add into pdir
      uint32_t *new_page_table = (uint32_t *) mm_allocphyspage();
      uint32_t *page_table = (uint32_t *) (0xFFC00000 + (pginf.pagetable * 0x1000)); // virt addr of page tbl

      kernel_page_dir[pginf.pagetable] = (uint32_t) new_page_table | 3; // add the new page table into the pdir
      page_table[pginf.page] = phys_address | 3; // map the page!
  	}
  return 1;
}

void initialise_paging()
{
	// Size of physical memory; set at 16MB
	uint32_t size_of_memory = 0x1000000;

	// Setup the space for the bitmap that tracks frame usage
	fb_write("Allocating space for frame bitmap...");
	total_frames = size_of_memory / 0x1000;
	frames = (uint32_t*)kmalloc_a(INDEX_FROM_BIT(total_frames));
	memset(frames, 0, INDEX_FROM_BIT(total_frames));

	// Setup the kernal page_directory
	fb_write("Allocating space for kernel_page_directory...");
	uint32_t *initial_kernel_page_dir = (uint32_t *)kmalloc_a(0x1000);
	memset(initial_kernel_page_dir, 0, 0x1000);
	fb_write("Address of kernel_page_directory: ");
	fb_write_hex(initial_kernel_page_dir);

	fb_write("Allocate space for first page_table...");
	uint32_t *first_page_table = (uint32_t *)kmalloc_a(0x1000);
	initial_kernel_page_dir[0] = first_page_table;
	initial_kernel_page_dir[0] += 0x3;
	fb_write("Address of first_page_directory: ");
	fb_write_hex(initial_kernel_page_dir[0]);

	fb_write("Allocate space for fourth page_table...");
	uint32_t *third_page_table = (uint32_t *)kmalloc_a(0x1000);
	initial_kernel_page_dir[3] = third_page_table;
	initial_kernel_page_dir[3] += 0x3;
	fb_write("Address of first_page_directory: ");
	fb_write_hex(initial_kernel_page_dir[3]);
	// Map last 4mb of (max) virtual memory to page directory (recursive)
	//kernel_page_dir[1023] = 0xFFFFF000;

	// We need to identity map (virtual == physical) the kernel memory used
	// so far or the kernel will be lost after turning on paging;
	// We will identity map the first 2MB of memory. This only requires setting
	// up a single (first) page.

	fb_write("Identity mapping kernel memory...");
	int i = 0;
	uint32_t j = 0;
	while (i < 255)
	{
		int frame_mask = j | 0x3;
		first_page_table[i] = frame_mask;
		set_frame(j);
		i += 1;
		j += 0x1000;
	}

	i = 256;
	j = 0xd00000;
	while (i < 767)
	{
		int frame_mask = j | 0x3;
		third_page_table[i] = frame_mask;
		set_frame(j);
		i += 1;
		j += 0x1000;
	}

	i = 0;
	j = 0;
	while (i < 256)
	{
		int frame_mask = j | 0x3;
		third_page_table[i] = frame_mask;
		set_frame(j);
		i += 1;
		j += 0x1000;
	}

	// Before we enable paging, we must register our page fault handler.
  install_irq_handler(14, page_fault);

  // Now, enable paging!
	fb_write("Enabling paging by loading CR3 with: ");
	fb_write_hex(initial_kernel_page_dir);
  switch_page_directory(initial_kernel_page_dir);
	fb_init(1);
	initial_kernel_page_dir[0]=0;
}

void switch_page_directory(uint32_t *dir)
{
    current_page_dir = dir;
		fb_write("\n Loading CR3: ");
		fb_write_hex(dir);
    asm volatile("mov %0, %%cr3":: "r"(dir));
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
