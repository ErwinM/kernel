/** descriptor_tables.c:
	* Sets up the GDT for entering protected mode. Also sets up
	* an IDT table
	*/

#include "common.h"
#include "gdt_flush.h"

static void gdt_set_entry( uint16_t entry_num, uint32_t limit, uint32_t base, uint8_t access_byte, uint8_t gran);
static void init_gdt();

struct gdt_entry
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access_byte;
	uint8_t limit_high_and_flags;
	uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr_struct
{
	uint16_t limit; 	// last 16 bits of all segment limits
	uint32_t base; 		// address of first GDT entry
}__attribute__((packed));

/* Set up scaffolding for 3 gdt entries */
/* and the special GDT pointer */

struct gdt_entry gdt[3];
struct gdt_ptr_struct gdt_ptr;

void init_descriptor_tables()
{
	init_gdt();
}

static void init_gdt()
{
		gdt_ptr.limit = (sizeof(struct gdt_entry) * 3) - 1; // 3 entries in our table
		gdt_ptr.base = (uint32_t)&gdt;

		gdt_set_entry(0, 0, 0, 0, 0);
		gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9a, 0xcf);
		gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xcf);

		gdt_flush((uint32_t)&gdt_ptr);
}

static void gdt_set_entry( uint16_t entry_num, uint32_t limit, uint32_t base, uint8_t access_byte, uint8_t gran)
{
	gdt[entry_num].base_low = (base & 0xFFFF); // only first 16 bits
	gdt[entry_num].base_mid = ((base >> 16) & 0xFF);
	gdt[entry_num].base_high = ((base >> 24) & 0xFF);

	gdt[entry_num].limit_low = (limit & 0xFFFF); // only first 16 bits
	gdt[entry_num].limit_high_and_flags = ((limit >> 16) & 0x0F);
	gdt[entry_num].limit_high_and_flags |= (gran & 0xF0);
	gdt[entry_num].access_byte = access_byte;
}
