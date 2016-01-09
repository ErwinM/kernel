#include "common.h"
#include "write.h"
#include "descriptor_tables.h"

int kmain(void)
{

	//const char *str = "erwin's first kernel";
	fb_init(0);
	fb_clear();
	fb_write("Setting up Global Descriptor Table...");
	init_gdt();
	fb_write("Success!\n");

	fb_write("Setting up Interrupt Descriptor Table...");
	init_idt();
	fb_write("Success!\n");

	//asm volatile ("int $0x3");
	//asm volatile ("int $0x4");


	fb_write("Initialising timer...");
	init_timer(50);
	fb_write("Success!\n");

	fb_write("Initialising paging...");
	initialise_paging();
  fb_write("Hello, paging world!\n");


	int32_t *tmp = (uint32_t *)0xcb8000;
	//fb_write_hex(*tmp);
  //uint32_t *ptr = (uint32_t*)0xA0000000;
  //uint32_t do_page_fault = *ptr;

  return 0;

}
