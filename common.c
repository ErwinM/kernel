// common.c -- Defines some global functions.

#include "common.h"


// Write len copies of val into dest.
void memset(uint8_t *dest, uint8_t val, uint32_t len)
{
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

void memcpy(uint16_t *dest, uint16_t *src, uint32_t len)
{
	const uint16_t *sp = (const uint16_t *)src;
  uint16_t *dp = (uint16_t *)dest;
  for(; len != 0; len--) *dp++ = *sp++;
}

extern void panic(const char *message, const char *file, uint32_t line)
{
    // We encountered a massive problem and have to stop.
    asm volatile("cli"); // Disable interrupts.
    fb_write("PANIC(");
    fb_write(message);
    fb_write(") at ");
    fb_write(file);
    fb_write(":");
    fb_write_dec(line);
    fb_write("\n");
    // trigger Bochs debug mode (magic instruction)
		asm volatile("xchg %bx, %bx");}

extern void panic_assert(const char *file, uint32_t line, const char *desc)
{
    // An assertion failed, and we have to panic.
    asm volatile("cli"); // Disable interrupts.

    fb_write("ASSERTION-FAILED(");
    fb_write(desc);
    fb_write(") at ");
    fb_write(file);
    fb_write(":");
    fb_write_dec(line);
    fb_write("\n");
    // trigger Bochs debug mode (magic instruction)
		asm volatile("xchg %bx, %bx");
		//for(;;);
}
