// implementation of file descriptor
#include "spinlock.h"
#include "common.h"
#include "param.h"
#include "file.h"
#include "defs.h"

struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;


// Locate and return an empty slot in the file table
int filealloc(void)
{
	struct file *fp;
	struct inode *ip;

	fp = &ftable.file;
	while(fp->ref > 0 && fp < &ftable.file[NFILE])
		fp++;
	if (fp->ref > 0)
		PANIC("fdalloc: cannot locate free file slot");
	fp->ref = 1;
	return fp;
}


int filewrite(struct file *f, char *addr, int n)
{
	if (f->type = FD_INODE) {

	}
	PANIC("filewrite: writing to unknown type");
}



int fileread(struct file *f, char *addr, int n)
{
  int r;
	//kprintf("fileread: type: %d", f->type);
  if(f->readable == 0)
    return -1;
  if(f->type == FD_PIPE)
		kprintf("fileread: pipe?!",0);
    //return piperead(f->pipe, addr, n);
  if(f->type == FD_INODE){
    ilock(f->ip);
    if((r = readi(f->ip, addr, f->off, n)) > 0)
      f->off += r;
    //iunlock(f->ip);
    return r;
  }
  PANIC("fileread");
}
