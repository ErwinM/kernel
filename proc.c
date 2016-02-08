
#include "common.h"
#include "proc.h"
#include "spinlock.h"
#include "mem_layout.h"
#include "mmu.h"
#include "param.h"


extern void forkret(void);
extern void trapret(void);
extern void startinitcode;
char *sip = &startinitcode;

struct {
	struct spinlock lock;
	struct proc proc[64]; // max 64 processes
} ptable;

int nextpid = 0;
struct proc *initproc;
struct proc *proc; // current running process
struct cpu maincpu;
struct cpu *mcpu = &maincpu;


void initptable()
{
	//...
}

static struct proc* allocproc()
{
	// find an empty ptable slot
	struct proc *p;
	char *sp;

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
	if ((p->kstack = kalloc()) == 0)
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
	memset(p->context, 0, sizeof *p->context);
	p->context->eip = (uint32_t)forkret;
	return p;
}

// Set up first user process. Copied from xv6
void userinit(void)
{
  struct proc *p;
  //extern char _binary_initcode_out_start[], _binary_initcode_out_size[];

  p = allocproc();
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    PANIC("userinit: out of memory?");

	inituvm(p->pgdir, sip, 1024);
	kprintf("userinit: initcode was linked at: %h", sip);
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
  //p->cwd = namei("/");
  p->state = RUNNABLE;
}
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
	kprintf("forkret: returning to trapret");
}
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void scheduler(void)
{
  struct proc *p;

  for(;;){
    // Enable interrupts on this processor.
    sti();
		fb_write("scheduler: IRQ enabled.\n");
    // Loop over process table looking for process to run.
    //acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      proc = p;
			fb_write("scheduler: switching context.\n");
      switchuvm(p);
      p->state = RUNNING;
			fb_write("scheduler: switching process.\n");
      swtch(&mcpu->scheduler, proc->context);
			fb_write("it doesnt get here...");
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
    }
    //release(&ptable.lock);

  }
}
