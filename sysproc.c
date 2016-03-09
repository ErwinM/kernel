#include "defs.h"



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
	int sz, n;

	if(n = argint(0, &sz) < 0)
		PANIC("sys_brk: no arguments");

}
