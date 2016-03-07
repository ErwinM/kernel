#include "defs.h"
#include "trap.h"
#include "x86.h"
#include "timer.h"

extern struct proc *cp;

void trap(struct trapframe *tf)
{
	if(tf->trapno != 32)
		//kprintf("Trapno: %d", tf->trapno);
	if(tf->trapno == T_SYSCALL){
		//kprintf("trap: SYSCALL: %d\n", tf->eax);
		cp->tf = tf;
		syscall();
		return;
	}
	switch(tf->trapno){
	case T_IRQ0 + IRQ_TIMER:
	 	timer_callback();
		EOI(tf->trapno);
		break;
	case T_PGFLT:
		page_fault(tf->err);
		EOI();
		break;
	case T_IRQ0 + IRQ_7:
		spurious(tf->trapno);
		break;
	case T_IRQ0 + IRQ_KBD:
		//spurious(tf->trapno);
		consoleintr();
		EOI(tf->trapno);
		return;
	default:
		kprintf("Unhandled trapno: %d.\n", tf->trapno);
		kprintf("Errno: %d", tf->err);
		PANIC("TRAP");
	}


	// this calls scheduler on timer interrupt
	if(cp->state == RUNNING && tf->trapno == T_IRQ0 + IRQ_TIMER)
		yield();
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
	if (1){//trapno >= 32){
		if(trapno !=32)
		outb(0x20, 0x20);
	}
}
