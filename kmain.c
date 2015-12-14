#include "common.h"
#include "write.h"
#include "descriptor_tables.h"

void kmain(void)
{

	//const char *str = "erwin's first kernel";
	fb_clear();
	fb_write("Setting up Global Descriptor Table...\n");
	init_gdt();
	fb_write("Setting up Interrupt Descriptor Table...\n");
	init_idt();
	fb_write_dec(12);
	asm volatile ("int $0x3");
	asm volatile ("int $0x4");
	asm volatile("sti");
	init_timer(50); 
}
