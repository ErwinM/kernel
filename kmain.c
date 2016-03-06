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

extern struct inode* dirlookup(struct inode*, char*);

int kmain(struct multiboot *mboot_ptr)
{
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
	//init_timer(50); // setup timer interrupt handler

	// up till now paging is still off, lets turn it on
	initpaging();
	initkheap(); // Create the kernel heap; without it you get stuck quickly
	iinitrd(initrd); // initialise ramdsk

	struct inode *root;
	struct file *fd;
	struct dirent dd[10];
	char *data[512];




	userinit();
	scheduler();

	fb_write("EXECUTION FINISHED.\n");
	return 0;
}
