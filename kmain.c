#include "common.h"
#include "write.h"
#include "descriptor_tables.h"
#include "ordered_list.h"

extern *kheap;

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

	fb_write("Initialising paging...\n");
	initialise_paging();
  fb_write("Hello, paging world!\n");


	uint32_t *a1 = alloc(8, 0, kheap);
	fb_printf("a1: %h", a1);
	uint32_t *b1 = alloc(8, 1, kheap);
	fb_printf("b1: %h", b1);
	uint32_t *c1 = alloc(8, 0, kheap);
	fb_printf("c1: %h", c1);
	//free(b1, kheap);
	//uint32_t *c1 = alloc(7, 0, kheap);
	//fb_printf("c1: %h", c1);

	fb_write("finished.");
  return 0;
}
