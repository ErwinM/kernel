// proc.h all structures that deal with processes...

#ifndef INCLUDE_PROC_H
#define INCLUDE_PROC_H

#include "common.h"
#include "x86.h"
#include "mmu.h"
#include "param.h"
#include "mem_layout.h"

// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebx;
  uint32_t ebp;
  uint32_t eip;
};

struct cpu {
  //uchar id;                    // Local APIC ID; index into cpus[] below
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  //volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?

  // Cpu-local storage variables; see below
  struct cpu *cpu;
  struct proc *proc;           // The currently-running process.
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
  uint32_t sz;                     // Size of process memory (bytes)
  uint32_t *pgdir;                // Page table
  char *kstack;                // PHYSICAL address of bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  //struct file *ofile[NOFILE];  // Open files
  //struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)

};
#endif
