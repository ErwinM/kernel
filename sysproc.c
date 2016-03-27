#include "defs.h"

extern struct proc *cp;


int sys_fork()
{
	// no need to grab arguments because there are none
	int i;
	i = fork();
	kprintf("sys_fork: return: %d", i);
	return i;
}

int sys_sbrk()
{
	int n, addr;

	kprintf("sys_sbrk: reached.");
	kprintf("sys_brk: proc-sz: %x", cp->sz);
	if(argint(0, &n) < 0)
		PANIC("sys_brk: no arguments");

	addr = cp->sz;

	kprintf("sys_brk: grow by: %x", n);

	if(growproc(n) < 0)
		PANIC("sys_brk:growproc failed");

	return addr;
}

int sys_wait()
{
	// just put calling process to sleep for now
	return wait();
}

int sys_exit()
{
	return exit();
}
