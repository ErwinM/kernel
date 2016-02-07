#ifndef INCLUDE_X86_H
#define INCLUDE_X86_H

#include "common.h"

// Layout of the trap frame built on the stack by the
// hardware and by trapasm.S, and passed to trap().
struct trapframe {
  // registers as pushed by pusha - Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t oesp;      // useless & ignored
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;

  // rest of trap frame
  uint16_t gs;
  uint16_t padding1;
  uint16_t fs;
  uint16_t padding2;
  uint16_t es;
  uint16_t padding3;
  uint16_t ds;
  uint16_t padding4;
  uint32_t trapno;

  // below here defined by x86 hardware
  uint32_t err;
  uint32_t eip;
  uint16_t cs;
  uint16_t padding5;
  uint32_t eflags;

  // below here only when crossing rings, such as from user to kernel
  uint32_t esp;
  uint16_t ss;
  uint16_t padding6;
};

// load GDT
static inline void
lgdt(struct segdesc *p, int size)
{
  volatile uint16_t pd[3];

  pd[0] = size-1;
  pd[1] = (uint32_t)p;
  pd[2] = (uint32_t)p >> 16;

  asm volatile("lgdt (%0)" : : "r" (pd));
}
// load task segment register
static inline void
ltr(uint16_t sel)
{
  asm volatile("ltr %0" : : "r" (sel));
}

static inline uint32_t
readeflags(void)
{
  uint32_t eflags;
  asm volatile("pushfl; popl %0" : "=r" (eflags));
  return eflags;
}

static inline void
cli(void)
{
  asm volatile("cli");
}

static inline void
sti(void)
{
  asm volatile("sti");
}

static inline uint32_t
xchg(volatile uint32_t *addr, uint32_t newval)
{
  uint32_t result;

  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1" :
               "+m" (*addr), "=a" (result) :
               "1" (newval) :
               "cc");
  return result;
}

static inline uint32_t
rcr2(void)
{
  uint32_t val;
  asm volatile("movl %%cr2,%0" : "=r" (val));
  return val;
}

static inline void
lcr3(uint32_t val)
{
  asm volatile("movl %0,%%cr3" : : "r" (val));
}

static inline void
enablepag(void)
{
	uint32_t cr0;
	asm volatile("mov %%cr0, %0": "=r"(cr0));
	cr0 |= 0x80000000; // Enable paging!
	asm volatile("mov %0, %%cr0":: "r"(cr0));
}
#endif
