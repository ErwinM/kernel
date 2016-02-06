
#include "common.h"
#include "proc.h"
#include "spinlock.h"
#include "mem_layout.h"
#include "param.h"

struct {
	struct spinlock lock;
	struct proc proc[64]; // max 64 processes
} ptable;

extern void forkret(void);
extern void trapret(void);
int nextpid = 0;


void initptable()
{
	//...
}

static struct proc* allocproc()
{
	// find an empty ptable slot
	struct proc *p;
	uint32_t *sp;

	for (p = ptable.proc ; p < &ptable.proc[64] ; p++ )
	{
		if(p->state == UNUSED)
			goto found;
			return 0;
	}
found:
	p->state = EMBRYO;
	p->pid = nextpid++;
	// allocate memory for kernel_stack
	if ((p->kstack = KSTACK_BOTTOM) == 0) // should use allocpys to see if there is atleast one free page
		PANIC("ALLOCPROC: out of memory!");
	sp = p->kstack + KSTACK_SIZE;
	// leave room for the trapframe
	sp -= sizeof(*p->tf);
	p->tf = (struct trapframe *)sp;

	// Set up new context to start executing at forkret,
	// which returns to trapret.
	sp -= 4;
	*(uint32_t*)sp = (uint32_t)trapret;
	sp -= sizeof(*p->context);
	p->context = (struct context*)sp;
	memset(p->context, 0, sizeof(*p->context));
	p->context->eip = (uint32_t)forkret;
	return p;
}
/*
// Set up first user process. Copied from xv6
void userinit(void)
{
  struct proc *p;
  //extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  initproc = p;
  if((p->pgdir = setupkvm()) == 0) // we need to implement this
    panic("userinit: out of memory?");

	inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
	
}
*/
// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  //release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    //iinit(0);
    //initlog(0);
  }
  // Return to "caller", actually trapret (see allocproc).
}
