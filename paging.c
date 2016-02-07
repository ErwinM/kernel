// paging.c:
// includes kernel memory allocation functions and paging code

#include "paging.h"
#include "common.h"
#include "write.h"
#include "alloc.h"
#include "param.h"
#include "mmu.h"

extern uint32_t end;

pageinfo mm_virtaddrtopageindex(uint32_t virtaddr);
void setup_pt(uint32_t *page_table, int index_start, int index_end, uint32_t physical_start);
page_dir_t* setupkvm();

// The kernel's page directory
//page_dir_t *kpd;
pte_t *kpgdir;
uint32_t *frames; // pointer to the bitmap used to map frames
uint32_t total_frames;

uint32_t placement_address = (uint32_t)&end;

uint32_t kmalloc(uint32_t size, int align)
{
	// Allocates memory starting directly at the end of loaded kernel
	// This memory is permenantly allocated and cannot be freed!
	if (align == 1 && (placement_address & ~0xFFFFF000)) // if address is not 4k-aligned
	{
		// Align it
		placement_address &= 0xFFFFF000;
		placement_address += 0x1000;
	}
	uint32_t placed_at = placement_address;
	kprintf("KMALLOC: requested: %d.", size);
	kprintf(" Placed at: %h.\n", placed_at);
	placement_address += size;
	return placed_at;
}

uint32_t kmalloc_a(uint32_t size)
{
	return kmalloc(size, 1);
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
				//fb_write("\n");
				//fb_write_hex(frames[i]);
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
									//fb_write("first free frame idx: ");
									//fb_write_dec(i*4*8+j);
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

uint32_t* walkpagedir(page_dir_t *pgdir, uint32_t vaddr)
{
	uint32_t *pgtable;
	pageinfo pginf = mm_virtaddrtopageindex(vaddr);
	pgtable = (uint32_t*)((uint32_t)pgdir->pde[pginf.pagetable] & 0xFFFFF000);
	return pgtable[pginf.page] & 0xFFFFF000;
}

void mappage(uint32_t *pgdir, uint32_t paddr, uint32_t vaddr, int perm)
{
	// DOES NOT SET PHYS PAGE BITMAP!
	pageinfo pginf = mm_virtaddrtopageindex(vaddr);
	uint32_t *pgtable;
	if (pgdir[pginf.pagetable] & 1){
		// pgtable exisits
		pgtable = (uint32_t*)(pgdir[pginf.pagetable] & 0xFFFFF000);
		if(pgtable[pginf.page] & 1){
			kprintf("vaddr: %h", vaddr);
			PANIC("mappage: vaddr already mapped!");
		}
	} else {
		// pgtable does not exist, make it
		fb_write("PT does not exist!");
		if((pgtable = kalloc()) == 0){
			PANIC("mappage: kalloc returned 0");
		}
		pgdir[pginf.pagetable] = (uint32_t)pgtable | perm | PTE_P;
		memset(pgtable, 0, PGSIZE);
	}
	pgtable[pginf.page] = paddr | perm | PTE_P;
	set_frame(paddr);
}

uint32_t mm_allocphyspage()
{
	//fb_write("Allocating page...");
	uint32_t idx = first_free_frame(); // idx is now the index of the first free frame
	uint32_t phys_address = (idx * 0x1000);
	set_frame(phys_address); // this frame is now ours!
	//memset(phys_address, 0, 4096); // need to use virtual address!
	//kprintf("ALLOCPHYSPAGE: Allocated phys_addr: %h", phys_address);
	return (phys_address);
}
/*
uint32_t mm_mappage(uint32_t phys_address, uint32_t virt_address)
{
	kprintf("MAPPAGE: mapping phys: %h", phys_address);
	kprintf(" to virt: %h.\n", virt_address);
	kprintf("MAPPAGE: kernel_page_dir: %h", kpd);
	pageinfo pginf = mm_virtaddrtopageindex(virt_address); // get the PDE and PTE indexes for the addr
	//kprintf("MAPPAGE: PT: %d", pginf.pagetable);
	//kprintf("kernel_page_dir[3]: %h", kpd->pde[pginf.pagetable]);
	if(((uint32_t)kpd->pde[pginf.pagetable]) & 1)
	{
      // page table exists.
			//fb_write("MAPPAGE: PT exists!");
      uint32_t *page_table = (uint32_t *) (0xFFC00000 + (pginf.pagetable * 0x1000)); // virt addr of page table
			kprintf("MAPPAGE: mapped PT address: %h", page_table);
			if(!page_table[pginf.page] & 1)
			{
          // page isn't mapped
          page_table[pginf.page] = phys_address | 3;
      }
			else
			{
          // page is already mapped
					fb_printf("attempting to map table: %d", pginf.pagetable);
					fb_printf("an page: %d", pginf.page);
					PANIC("PAGE ALREADY MAPPED");
      }
  	}
		else
		{
			// page table doesn't exist, so alloc a page and add into pdir
			fb_write("PT does not exist..");
			uint32_t *phys_addr_page_table = (uint32_t *) mm_allocphyspage();
      uint32_t *page_table = (uint32_t *) (0xFFC00000 + (pginf.pagetable * 0x1000)); // virt addr of page tbl

      kpd->pde[pginf.pagetable] = (uint32_t) phys_addr_page_table | 3; // add the new page table into the pdir
      page_table[pginf.page] = phys_address | 3; // map the page!
  	}
	return 1;
}

void mm_unmappage(unsigned long virt_address)
{
	pageinfo pginf = mm_virtaddrtopageindex(virt_address);

  if((uint32_t)kpd->pde[pginf.pagetable] & 1){
    int i;
    unsigned long *page_table = (unsigned long *) (0xFFC00000 + (pginf.pagetable * 0x1000));
    if(page_table[pginf.page] & 1){
        // page is mapped, so unmap it and free the pysical page
        page_table[pginf.page] = 2; // r/w, not present
				clear_frame(page_table[pginf.page] & 0xFFFFF000);
  	}

	  // check if there are any more present PTEs in this page table
	  for(i = 0; i < 1024; i++){
	      if(page_table[i] & 1) break;
	  }

    // if there are none, then free the space allocated to the page table and delete mappings
    if(i == 1024){
        //clear_frame(kernel_page_dir[pginf.pagetable] & 0xFFFFF000);
        kpd->pde[pginf.pagetable] = 2;
    }
  }
}
*/
void initpaging()
{
	fb_write("Initialising paging...\n");
	// Size of physical memory; set at 16MB
	uint32_t size_of_memory = 0x1000000;

	// Setup the space for the bitmap that tracks frame usage
	fb_write("Allocating space for frame bitmap...\n");
	total_frames = size_of_memory / 0x1000;
	frames = (uint32_t*)kmalloc_a(INDEX_FROM_BIT(total_frames));
	memset(frames, 0, INDEX_FROM_BIT(total_frames));

	// Setup the kernal page_directory in critical memory region
	fb_write("Allocating space for kernel_page_directory...\n");
	kpgdir = (uint32_t *)kmalloc_a(0x1000);
	memset(kpgdir, 0, 0x1000);
	kprintf("Address of kernel_page_directory: %h\n", kpgdir);

	fb_write("Allocate space for first page_table...\n");
	uint32_t *first_page_table = (uint32_t *)kmalloc_a(0x1000);
	*kpgdir = (uint32_t)first_page_table + 3;
	fb_printf("Address of first_page_directory: %h\n", *kpgdir);

	fb_write("Allocate space for fourth page_table...");
	uint32_t *third_page_table = (uint32_t *)kmalloc_a(0x1000);
	kpgdir[3] = (uint32_t)third_page_table + 3;
	fb_printf("Address of fourth_page_directory: %h\n", kpgdir[3]);

	fb_write("Identity mapping kernel memory...");
	// Temporarily identity map first MB of physical memory or kernel will crash
	// due to unavailable ROM memory
	setup_pt(first_page_table, 0, 255, 0 );

	// Map first Mb physical to 0xc00000; 0xc00000 is the start of the 3rd PT
	setup_pt(third_page_table, 0, 255, 0);

	// Identity map the kernel code and critical data area:
	// 0xd00000 up to 0xf00000
	setup_pt(third_page_table, 256, 768, 0xd00000);

  // Now, enable paging!
	fb_printf("Enabling paging by loading CR3 with: %h", kpgdir);
  switch_page_directory(kpgdir);
	enablepag();
	fb_write("..enabled.\n");
	fb_init(1); // Redirect pointer to video memory
	initkheap(); // Create the kernel heap; without it you get stuck quickly
	// we can now use 'normal' paging functions to build a new pagedir
	//page_dir_t *kpgdir = setupkvm();
	//switch_page_directory(kpgdir);

	//kpd->pde[0] = 0; // free up first 1mb of linear memory space

}

void switch_page_directory(uint32_t *dir)
{
    //currpgdir = dir;
    asm volatile("mov %0, %%cr3":: "r"(dir));
}

void enablepag()
{
	uint32_t cr0;
	asm volatile("mov %%cr0, %0": "=r"(cr0));
	cr0 |= CR0_PG; // Enable paging!
	asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void page_fault(uint32_t err)
{
   // A page fault has occurred.
   // The faulting address is stored in the CR2 register.
   uint32_t faulting_address;
   asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

   // The error code gives us details of what happened.
   int present   = !(err & 0x1); // Page not present
   int rw = err & 0x2;           // Write operation?
   int us = err & 0x4;           // Processor was in user-mode?
   int reserved = err & 0x8;     // Overwritten CPU-reserved bits of page entry?
   int id = err & 0x10;          // Caused by an instruction fetch?

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

void setup_pt(uint32_t *page_table, int index_start, int index_end, uint32_t physical_start)
{
	while (index_start <= index_end)
	{
		int frame_mask = physical_start | 0x3;
		page_table[index_start] = frame_mask;
		set_frame(physical_start);
		index_start += 1;
		physical_start += 0x1000;
	}
}
