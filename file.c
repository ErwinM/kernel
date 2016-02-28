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
		//kprintf("fileread: recognized INODE type",0);
    //ilock(f->ip);
    if((r = readi(f->ip, addr, f->off, n)) > 0)
      f->off += r;
    //iunlock(f->ip);
    return r;
  }
  PANIC("fileread");
}