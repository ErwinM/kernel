/**
 	* isr.c -- High level interrupt service routines and interrupt request handlers.
 	* Part of this code is modified from Bran's kernel development tutorials.
 	* Rewritten for JamesM's kernel development tutorials.
	*/

#include "common.h"
#include "isr.h"
#include "write.h"

isr_t interrupt_handlers[256];

/* Each of the IRQ ISRs point to this function, rather than
*  the 'fault_handler' in 'isrs.c'. The IRQ Controllers need
*  to be told when you are done servicing them, so you need
*  to send them an "End of Interrupt" command (0x20). There
*  are two 8259 chips: The first exists at 0x20, the second
*  exists at 0xA0. If the second controller (an IRQ from 8 to
*  15) gets an interrupt, you need to acknowledge the
*  interrupt at BOTH controllers, otherwise, you only send
*  an EOI command to the first controller. If you don't send
*  an EOI, you won't raise any more IRQs */
void irq_handler(regs_t regs)
{
		//fb_write("common irq handler triggered..");

		isr_t handler = interrupt_handlers[regs.int_no - 32];

		if (handler)
		{
			handler(regs);
		}
    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (regs.int_no >= 40)
    {
        outb(0xA0, 0x20);
    }
    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outb(0x20, 0x20);
}

// This gets called from our ASM interrupt handler stub.
void fault_handler(regs_t regs)
{

	isr_t handler = interrupt_handlers[regs.int_no];

	if (handler)
	{
		handler(regs);
	}
	else
	{
  	fb_write("unhandled interrupt: ");
    fb_write_dec(regs.int_no);
    fb_write('\n');
  }
}

/* This installs a custom IRQ handler for the given IRQ */
void install_irq_handler(int irq, void (*handler)(regs_t r))
{
    interrupt_handlers[irq] = handler;
}

/* This clears the handler for a given IRQ */
void uninstall_irq_handler(int irq)
{
    interrupt_handlers[irq] = 0;
}
