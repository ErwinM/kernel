#include "common.h"
#include "write.h"
#include "descriptor_tables.h"
#include "ordered_list.h"
#include "alloc.h"

int kmain(struct multiboot *mboot_ptr)
{
	/**
	{
  kinit1(end, P2V(4*1024*1024)); // phys page allocator
  kvmalloc();      // kernel page table
  mpinit();        // collect info about this machine
  --lapicinit();
  seginit();       // set up segments
  cprintf("\ncpu%d: starting xv6\n\n", cpu->id);
  picinit();       // interrupt controller
  --ioapicinit();    // another interrupt controller
  consoleinit();   // I/O devices & their interrupts
  --uartinit();      // serial port
  pinit();         // process table
  tvinit();        // trap vectors
  binit();         // buffer cache
  fileinit();      // file table
  ideinit();       // disk
  if(!ismp)
    timerinit();   // uniprocessor timer
  startothers();   // start other processors
  kinit2(P2V(4*1024*1024), P2V(PHYSTOP)); // must come after startothers()
  userinit();      // first user process
  // Finish setting up this processor in mpmain.
  mpmain();
}
	*/
	//const char *str = "erwin's first kernel";
	fb_clear();  // screen
	fb_init(0);
	initgdt();  // setup gdt
	init_idt(); // setup the interrup tables
	init_timer(50); // setup timer interrupt handler

	// up till now paging is still off, lets turn it on
	initpaging();


	fb_write("EXECUTION FINISHED.\n");
	return 0;
}
