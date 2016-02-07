#include "mmu.h"
#include "x86.h"
#include "proc.h"

extern struct cpu *mcpu;

// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

void pushcli(void)
{
  int eflags;

  eflags = readeflags();
  cli();
  if(mcpu->ncli++ == 0)
    mcpu->intena = eflags & FL_IF;
}

void popcli(void)
{
  if(readeflags() & FL_IF)
    PANIC("popcli: interruptible");
  if(--mcpu->ncli < 0)
    PANIC("popccli: ncli < 0");
  if(mcpu->ncli == 0 && mcpu->intena)
    sti();
}
