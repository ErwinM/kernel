#include "defs.h"
#include "trap.h"
#include "x86.h"
#include "timer.h"

void trap(struct trapframe *tf)
{
	//kprintf("Trapno: %d", tf->trapno);
	if(tf->trapno == T_SYSCALL){
		fb_write("SYSCALL!");
	}
	switch(tf->trapno){
	case T_IRQ0 + IRQ_TIMER:
	 	timer_callback();
		EOI();
		break;
	case T_PGFLT:
		page_fault(tf->err);
		EOI();
		break;
	case T_IRQ0 + IRQ_7:
	case T_IRQ0 + IRQ_KBD:
		spurious(tf->trapno);
		break;
	default:
		kprintf("Unhandled trapno: %d.\n", tf->trapno);
		kprintf("Errno: %d", tf->err);
		PANIC("TRAP");
	}
}

void spurious(uint32_t trapno)
{
	outb(0x20, 0x0B);
	unsigned char irr = inb(0x20);
	if ( irr & 0x80 ) {
		EOI(trapno);
	}
}

void EOI(uint32_t trapno)
{
	// IRQ involving slave (meaning IRQ8 - 15), send EOI to slave
	if (trapno <= 47 && trapno >= 40){
			outb(0xA0, 0x20);
	}
	// Any IRQ, send EOI to master
	if (trapno >= 32){
		outb(0x20, 0x20);
	}
}
