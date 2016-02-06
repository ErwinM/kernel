
#include "common.h"
#include "proc.h"
#include "spinlock.h"
#include "mem_layout.h"

struct {
	struct spinlock lock;
	struct proc proc[64]; // max 64 processes
} ptable;

int nextpid = 0;

void initptable()
{
	//...
}

void allocproc()
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
	if ((p->kstack = mm_allocphyspage()) == 0)
		PANIC("OOM!");
	sp = p->kstack + KSTACK_SIZE;
	// leave room for the trapframe
	sp -= sizeof(*p->tf);
	p->tf = (struct trapframe *)sp;

	// Set up new context to start executing at forkret,
	// which returns to trapret.
	sp -= 4;
	*(uint32_t*)sp = (uint32_t)0x0; // DUMMY FOR TRAPRET
	sp -= sizeof(*p->context);
	p->context = (struct context*)sp;
	memset(p->context, 0, sizeof(*p->context));
	p->context->eip = (uint32_t)0x0;
	return p;
	// build page_table?
}

void userinit()
{
	// setup the entry
	// build first trap frame
}
