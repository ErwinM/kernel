/**
 	* isr.c -- High level interrupt service routines and interrupt request handlers.
 	* Part of this code is modified from Bran's kernel development tutorials.
 	* Rewritten for JamesM's kernel development tutorials.
	*/

#include "common.h"
#include "isr.h"
#include "write.h"

// This gets called from our ASM interrupt handler stub.
void fault_handler(regs_t regs)
{
   fb_write("recieved interrupt: ");
   fb_write_dec(regs.int_no);
   fb_put_char('\n');
}
