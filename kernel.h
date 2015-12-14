#ifndef INCLUDE_KERNEL_H
#define INCLUDE_KERNEL_H

/** outb:
 *	Sends the given data to the given I/O port. Defined in io.s
 *
 * @param port The I/O port to send the data to
 * @param data The data to send
 */

void outb(unsigned short port, unsigned char data);


 /** gdt_flush:
  *
  */
void gdt_flush(uint32_t pointer);

void idt_flush(uint32_t pointer);

#endif
