// paging.c:
// includes kernel memory allocation functions and paging code

#include "paging.h"
#include "common.h"
#include "write.h"
#include "alloc.h"
#include "param.h"
#include "mmu.h"


/*
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
