#include "defs.h"
#include "common.h"
#include "write.h"
#include "descriptor_tables.h"
#include "proc.h"
#include "multiboot.h"
#include "file.h"
#include "fs.h"

uint32_t placement_address;
struct multiboot mboothdr;
uint32_t *initrd;

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
	// grab and copy multiboot header (its somewhere in low memory)
	memcpy(&mboothdr, mboot_ptr, sizeof(struct multiboot));
	initrd = *((uint32_t*)mboothdr.mods_addr);
	uint32_t initrdend = *(uint32_t*)(mboothdr.mods_addr+4);
	placement_address = initrdend;

	kprintf("kmain: initrd: %h", initrd);


	initgdt();  // setup gdt
	init_idt(); // setup the interrup tables
	init_timer(50); // setup timer interrupt handler

	// up till now paging is still off, lets turn it on
	initpaging();
	initkheap(); // Create the kernel heap; without it you get stuck quickly

	struct inode *root;
	struct file *fd;
	//struct dirent de;
	struct dirent dd[10];

	iinitrd(initrd);

	root = iget(1, 1);
	kprintf("kmain: loc of root: %h", root);
	fd->type = FD_INODE;
	fd->ref = 0;
	fd->readable = 1;
	fd->writable = 0;
	fd->ip = root;
	fd->off = 0;

	kprintf("kmain: fd->ip %h", fd->ip);

	int k, nr;
	k = listinitrd(root, &dd);
	for (nr = 0 ; nr < k ; nr++){
		kprintf("kmain: list: ",0);
		fb_write(dd[nr].name);
	}

	 //struct dirent *de = readinitrd(fd->ip, 0, sizeof(struct dirent));
	 //kprintf("kmain: de.name: ", 0);
	 //fb_write((*de).name);
/*
	 int k;
	 k = readi(root, &buf, 0, 100);
	 struct dirent *dd;
	 dd = (struct dirent*)buf;
	 kprintf("kmain: buf: %h", buf);
	 kprintf("kmain: &buf[0]: %h", &buf[0]);
	 kprintf("kmain: dd: %h", dd);

	 fb_write("kmain: readi: ");
	 fb_write(dd[0].name);

	/*
	while(fileread(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      //memmove(p, de.name, DIRSIZ);
      //p[DIRSIZ] = 0;
    //  fb_write("entry: ");
			//fb_write(buf);
		}
		*/
	//userinit();
	//scheduler();

	fb_write("EXECUTION FINISHED.\n");
	return 0;
}
