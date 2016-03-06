#include "common.h"
#include "defs.h"
#include "kbd.h"

void kbdgetc(void)
{

}

void consoleintr(void)
{
	int i;

	i = inb(KBSTATUS);

	kprintf("consoleintr: %h", i);
	sti();
}
