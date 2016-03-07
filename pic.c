#include "trap.h"

#define IOPIC1			0x20		/* IO base address for master PIC */
#define IOPIC2			0xA0		/* IO base address for slave PIC */
#define PIC_EOI		0x20

#define ICW1_ICW4				0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE			0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL			0x08		/* Level triggered (edge) mode */
#define ICW1_INIT				0x11		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

static uint8_t irqmask = 0xFFFF & ~(1 << 2); // master has irq2 (slave enabled)

void picsetmask(uint8_t mask)
{
	irqmask = mask;
	outb(IOPIC1+1, mask);
	outb(IOPIC2+1, mask >> 8);
}

void enableirq(uint8_t irq)
{
	picsetmask(irqmask << ~(1 << irq));
}

void initpic(void)
{
	unsigned char a1, a2;

 	outb(IOPIC1+1, 0xff); // mask all interrupts
 	outb(IOPIC1+2, 0xff);

	// setup PIC 1
	outb(IOPIC1, ICW1_INIT);  // starts the initialization sequence (in cascade mode)

	outb(IOPIC1+1, T_IRQ0);   // ICW2: Master PIC vector offset
	outb(IOPIC1+1, 4);				// ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(IOPIC1+1, ICW4_8086 +ICW4_AUTO);

	//setup PIC 2
	outb(IOPIC2, ICW1_INIT);  // starts the initialization sequence (in cascade mode)

	outb(IOPIC2+1, T_IRQ0+8);   // ICW2: Master PIC vector offset
	outb(IOPIC2+1, 2);				// ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(IOPIC2+1, ICW4_8086 +ICW4_AUTO);

	picsetmask(0);
	enableirq(1);
}
