/** descriptor_tables.c:
	* Sets up the GDT for entering protected mode. Also sets up
	* an IDT table
	*/

#include "common.h"
#include "kernel.h"
#include "descriptor_tables.h"
#include "mem_layout.h"
#include "mmu.h"

static void gdt_set_entry( uint16_t entry_num, uint32_t limit, uint32_t base, uint8_t access_byte, uint8_t gran);
static void idt_set_entry( uint16_t entry_num, uint32_t base, uint16_t sel, uint8_t access_byte );

struct gdt_entry
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access_byte;
	uint8_t limit_high_and_flags;
	uint8_t base_high;
}__attribute__((packed));

struct gdt_ptr_struct
{
	uint16_t limit; 	// last 16 bits of all segment limits
	uint32_t base; 		// address of first GDT entry
}__attribute__((packed));

struct idt_entry
{
	uint16_t base_low;
	uint16_t sel;
	uint8_t always_0;
	uint8_t access_byte;
	uint16_t base_high;
}__attribute__((packed));

struct idt_ptr_struct
{
	uint16_t limit;
	uint32_t base;
}__attribute__((packed));

/* Set up scaffolding for 3 gdt entries */
/* and the special GDT pointer */

//struct segdesc gdt[NSEGS];
//struct gdt_entry gdt[5];
struct gdt_ptr_struct gdt_ptr;
//struct cpu mcpu;

/* Setup 256 entries for the IDT */
/* and the special pointer */
struct idt_entry idt[256];
struct idt_ptr_struct idt_ptr;

/*
void initgdtOLD()
{
		fb_write("Setting up Global Descriptor Table...");
		gdt_ptr.limit = (sizeof(struct gdt_entry) * 5) - 1; // 5 entries in our table
		gdt_ptr.base = (uint32_t)&gdt;

		gdt_set_entry(0, 0, 0, 0, 0);
		gdt_set_entry(SEG_KCODE, 0, 0xFFFFFFFF, 0x9a, 0xcf);
		gdt_set_entry(SEG_KDATA, 0, 0xFFFFFFFF, 0x92, 0xcf);
		gdt_set_entry(SEG_UCODE, 0, 0xFFFFFFFF, 0xFa, 0xcf);
		gdt_set_entry(SEG_UDATA, 0, 0xFFFFFFFF, 0xF2, 0xcf);
		// todo: add TSSseg

		gdt_flush((uint32_t)&gdt_ptr);
		fb_write("Succes.\n");
}

static void gdt_set_entry( uint16_t entry_num, uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t gran)
{
	gdt[entry_num].base_low = (base & 0xFFFF); // only first 16 bits
	gdt[entry_num].base_mid = ((base >> 16) & 0xFF);
	gdt[entry_num].base_high = ((base >> 24) & 0xFF);

	gdt[entry_num].limit_low = (limit & 0xFFFF); // only first 16 bits
	gdt[entry_num].limit_high_and_flags = ((limit >> 16) & 0x0F);
	gdt[entry_num].limit_high_and_flags |= (gran & 0xF0);
	gdt[entry_num].access_byte = access_byte;
}
*/
static void idt_set_entry( uint16_t entry_num, uint32_t base, uint16_t sel, uint8_t access_byte )
{
	idt[entry_num].base_low = (base & 0xFFFF);
	idt[entry_num].base_high = ((base >> 16) & 0xFFFF);
	idt[entry_num].sel = sel;
	idt[entry_num].always_0 = 0;
	idt[entry_num].access_byte = access_byte;
}

void init_idt()
{
	fb_write("Setting up interrupt tables...");

	idt_ptr.limit = (sizeof(struct idt_entry) * 256 ) - 1;
	idt_ptr.base = (uint32_t)&idt;

	// Remap the irq table in the PICs.
  outb(0x20, 0x11);
  outb(0xA0, 0x11);
  outb(0x21, 0x20);
  outb(0xA1, 0x28);
  outb(0x21, 0x04);
  outb(0xA1, 0x02);
  outb(0x21, 0x01);
  outb(0xA1, 0x01);
  outb(0x21, 0x0);
  outb(0xA1, 0x0);

	// Clear the IDT table
	memset((uint8_t *)&idt, 0, sizeof(struct idt_entry) * 256);

	// Build new IDT table
	idt_set_entry(0, (uint32_t)isr0, 0x08, 0x8E);
	idt_set_entry(1, (uint32_t)isr1, 0x08, 0x8E);
	idt_set_entry(2, (uint32_t)isr2, 0x08, 0x8E);
	idt_set_entry(3, (uint32_t)isr3, 0x08, 0x8E);
	idt_set_entry(4, (uint32_t)isr4, 0x08, 0x8E);
	idt_set_entry(5, (uint32_t)isr5, 0x08, 0x8E);
	idt_set_entry(6, (uint32_t)isr6, 0x08, 0x8E);
	idt_set_entry(7, (uint32_t)isr7, 0x08, 0x8E);
	idt_set_entry(8, (uint32_t)isr8, 0x08, 0x8E);
	idt_set_entry(9, (uint32_t)isr9, 0x08, 0x8E);
	idt_set_entry(10, (uint32_t)isr10, 0x08, 0x8E);
	idt_set_entry(11, (uint32_t)isr11, 0x08, 0x8E);
	idt_set_entry(12, (uint32_t)isr12, 0x08, 0x8E);
	idt_set_entry(13, (uint32_t)isr13, 0x08, 0x8E);
	idt_set_entry(14, (uint32_t)isr14, 0x08, 0x8E);
	idt_set_entry(15, (uint32_t)isr15, 0x08, 0x8E);
	idt_set_entry(16, (uint32_t)isr16, 0x08, 0x8E);
	idt_set_entry(17, (uint32_t)isr17, 0x08, 0x8E);
	idt_set_entry(18, (uint32_t)isr18, 0x08, 0x8E);
	idt_set_entry(19, (uint32_t)isr19, 0x08, 0x8E);
	idt_set_entry(20, (uint32_t)isr20, 0x08, 0x8E);
	idt_set_entry(21, (uint32_t)isr21, 0x08, 0x8E);
	idt_set_entry(22, (uint32_t)isr22, 0x08, 0x8E);
	idt_set_entry(23, (uint32_t)isr23, 0x08, 0x8E);
	idt_set_entry(24, (uint32_t)isr24, 0x08, 0x8E);
	idt_set_entry(25, (uint32_t)isr25, 0x08, 0x8E);
	idt_set_entry(26, (uint32_t)isr26, 0x08, 0x8E);
	idt_set_entry(27, (uint32_t)isr27, 0x08, 0x8E);
	idt_set_entry(28, (uint32_t)isr28, 0x08, 0x8E);
	idt_set_entry(29, (uint32_t)isr29, 0x08, 0x8E);
	idt_set_entry(30, (uint32_t)isr30, 0x08, 0x8E);
	idt_set_entry(31, (uint32_t)isr31, 0x08, 0x8E);
	idt_set_entry(32, (uint32_t)irq0, 0x08, 0x8E);
  idt_set_entry(33, (uint32_t)irq1, 0x08, 0x8E);
  idt_set_entry(34, (uint32_t)irq2, 0x08, 0x8E);
  idt_set_entry(35, (uint32_t)irq3, 0x08, 0x8E);
  idt_set_entry(36, (uint32_t)irq4, 0x08, 0x8E);
  idt_set_entry(37, (uint32_t)irq5, 0x08, 0x8E);
  idt_set_entry(38, (uint32_t)irq6, 0x08, 0x8E);
  idt_set_entry(39, (uint32_t)irq7, 0x08, 0x8E);
  idt_set_entry(40, (uint32_t)irq8, 0x08, 0x8E);
  idt_set_entry(41, (uint32_t)irq9, 0x08, 0x8E);
  idt_set_entry(42, (uint32_t)irq10, 0x08, 0x8E);
  idt_set_entry(43, (uint32_t)irq11, 0x08, 0x8E);
  idt_set_entry(44, (uint32_t)irq12, 0x08, 0x8E);
  idt_set_entry(45, (uint32_t)irq13, 0x08, 0x8E);
  idt_set_entry(46, (uint32_t)irq14, 0x08, 0x8E);
  idt_set_entry(47, (uint32_t)irq15, 0x08, 0x8E);
	// SYS_CALL
	idt_set_entry(64, (uint32_t)isr64, 0x08, 0xEE);

	idt_flush((uint32_t)&idt_ptr);

	fb_write("Success!\n");
}
