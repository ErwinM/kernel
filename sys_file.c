// System calls for fs

#include "defs.h"
#include "param.h"


int argfd(uint32_t n, uint32_t *pfd, struct file **pf)
{
	int fd;
	struct file *f;

	if (argint(n, &fd) < 0)
		kprintf("argfd: cannot get argument",0);
		bbrk();
	if (fd < 0 || fd > NFILE)
		kprintf("argfd: got invalid fd",0);
		bbrk();
	if (pfd)
		pfd = fd;
	if (pf)
		pf = f;
	return 0;
}

// sys_write(fd, pointer to char buffer, number of chars)
int sys_write(void)
{
	kprintf("sys_write reached!",0);
	int n;
	struct file *f;
	char *p;

	if ( argfd(0,0,f) < 0 || argint(2, &n) || argptr(1, &p, n) )
		PANIC("sys_write: BAD arguments!");
	return filewrite(f, p , n);
}
