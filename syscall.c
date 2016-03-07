#include "defs.h"
#include "proc.h"
#include "syscall.h"

extern struct proc *cp;

int
fetchint(uint32_t addr, uint32_t *ip)
{
	//kprintf("fetchinit: addr: %h", addr);
	//kprintf("fetchinit: cp->sz: %h", cp->sz);
	//kprintf("fetchinit: cp->sz: %h", cp->tf->esp);

	if(addr >= cp->sz || addr+4 > cp->sz) {
		PANIC("fetchint");
		return -1;
	}
	//kprintf("addr: %h", addr);
	*ip = *(uint32_t*)(addr);
  return 0;
}

int
fetchstr(uint32_t addr, char **pp)
{
	char *s, *ep;

	//kprintf("fetchstr: addr: %h", addr);
	//kprintf("fetchstr: cp->sz: %h", cp->sz);
 	if(addr >= cp->sz) {
		PANIC("fetchstr cp-sz");
		return -1;
	}
 	*pp = (char*)addr;
 	ep = (char*)cp->sz;
	//kprintf("fetchstr: *pp",0);
	fb_write(*pp);
 	for(s = *pp; s < ep; s++)
		if(*s == 0)
			return s - *pp;
	PANIC("fetchstr");
	return -1;
}

//

int argint(uint32_t n, uint32_t *ip)
{
	fetchint(cp->tf->esp + 4 + 4*n, ip);
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
	char *buf;

  if(argint(n, &addr) < 0) {
		PANIC("argstr: no arguments");
		return -1;
	}
	return fetchstr(addr, pp);
}

// the actual sys call functions are found in sys_file, sys_proc and exec
extern int sys_write(void);
extern int sys_read();
extern int sys_exec(void);
extern int sys_fork(void);

// Syscall table
static int (*syscalls[])(void) = {
[SYS_write]   sys_write,
[SYS_exec]		sys_exec,
[SYS_fork]		sys_fork,
[SYS_read]		sys_read
};

int syscall(void)
{
	int num;
	num = cp->tf->eax; // sys call number is stored here
	if ( num > 0 && num < NELEM(syscalls) && syscalls[num] ) {
		cp->tf->eax = syscalls[num]();
		if(num == 1)
			kprintf("syscall: eax: %d", cp->tf->eax);
	} else {
		kprintf("syscall: unhandled syscall.",0);
		bbrk();
	}
}
