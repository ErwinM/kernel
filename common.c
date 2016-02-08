// common.c -- Defines some global functions.
#include "defs.h"
#include "common.h"


// Write len copies of val into dest.
void memset(void *dest, int val, uint32_t len)
{
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

void memcpy(void *dest, void *src, uint32_t len)
{
	const uint16_t *sp = (const uint16_t *)src;
  uint16_t *dp = (uint16_t *)dest;
  for(; len != 0; len--) *dp++ = *sp++;
}

void* memmove(void *dst, const void *src, uint32_t n)
{
  const char *s;
  char *d;

  s = src;
  d = dst;
  if(s < d && s + n > d){
    s += n;
    d += n;
    while(n-- > 0)
      *--d = *--s;
  } else
    while(n-- > 0)
      *d++ = *s++;

  return dst;
}

// Like strncpy but guaranteed to NUL-terminate.
char* safestrcpy(char *s, const char *t, int n)
{
  char *os;

  os = s;
  if(n <= 0)
    return os;
  while(--n > 0 && (*s++ = *t++) != 0)
    ;
  *s = 0;
  return os;
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
		asm volatile("xchg %bx, %bx");
}

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
