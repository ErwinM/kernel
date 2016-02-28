#include "defs.h"
#include "proc.h"
#include "syscall.h"

extern struct proc *cp;

int
fetchint(uint32_t addr, int *ip)
{
  if(addr >= cp->sz || addr+4 > cp->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

int
fetchstr(uint32_t addr, int **pp)
{
	char *s, *ep;

 	if(addr >= cp->sz)
		return -1;
 	*pp = (char*)addr;
 	ep = (char*)cp->sz;
 	for(s = *pp; s < ep; s++)
		if(*s == 0)
			return (int)s - (int)*pp;
 	return -1;
}

int argint(uint32_t n, uint32_t *ip)
{
	fetchint(cp->tf->esp + 4 + 4*n, *ip);
}

int
argptr(int n, char **pp, int size)
{
  int i;

  if(argint(n, &i) < 0)
    return -1;
  if((uint32_t)i >= cp->sz || (uint32_t)i+size > cp->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

// the actual sys call functions are found in sys_file, sys_proc and exec
extern int sys_write(void);

// Syscall table
static int (*syscalls[])(void) = {
[SYS_write]   sys_write
};

int syscall(void)
{
	int num;
	num = cp->tf->eax; // sys call number is stored here
	if ( num > 0 && num < NELEM(syscalls) && syscalls[num] ) {
		cp->tf->eax = syscalls[num]();
	} else {
		kprintf("syscall: unhandled syscall.",0);
		bbrk();
	}
}
