// System calls for fs

#include "defs.h"
#include "param.h"
#include "file.h"

extern struct proc *cp;

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

// Allocate a fd to a caller process in the ptable entry
// to indicate it has an open file
fd_t fdalloc(struct file *f)
{
	int fd;
	for(fd = 0; fd < NOFILE; fd++){
		if(cp->ofile[fd]==0){
			cp->ofile[fd] = f;
			return fd;
		}
	}
}


/*
The open system call is the first step a process must take to access the data in a
file. The syntax for the open system call is fd = open(pathname, flags, modes);
where pathname is a file name, flags indicate the type of open (such as for reading or writing),
and modes give the file permissions if the file is being created. The open system cal] returns an
integer' called the user file descripto
*/

//fd = open(pathname, omode);
fd_t sys_open(void)
{

	char *path;
	int omode, fd;
	struct file *f;
	struct inode *ip;

	if (argstr(0, &path) == 0 || argint(1, &omode) == 0)
		return -1;

	if ((ip = namei(path)) == 0)
		PANIC("sys_open: file not found");
	if ((f = filealloc()) == 0)
		PANIC("sys_open: can't allocate file table entry");
	if ((fd = fdalloc(f)) < 0)
		PANIC("sys_open: can't allocate file descriptor");
	f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
	return fd;
}
