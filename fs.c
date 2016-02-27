// implementation of the vfs that deal with i-nodes
#include "defs.h"
#include "common.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "file.h"

struct devsw devsw[3];
struct {
  struct spinlock lock;
  struct inode inode[NINODE];
} icache;

// Find the inode with number inum on device dev
// and return the in-memory copy. Does not lock
// the inode and does not read it from disk.
struct inode* iget(uint32_t dev, uint32_t inum)
{
	kprintf("iget: icache loc: %h", &icache);
	// check if already in inode cache, if so fetch it
	struct inode *ip, *empty;
	empty = 0;
	for (ip = &icache.inode[0] ; ip < &icache.inode[NINODE] ; ip++){
		if (ip->ref > 0 && ip->dev == dev && ip->inum == inum ) {
			ip->ref++;
			PANIC("HUH?");
			return ip;
		}
		if (empty == 0 && ip->ref == 0 ){
			// remember empty slot
			empty = ip;
			kprintf("iget: empty: %h", empty);
		}
	}
	// if not allocate a cache entry
	if (empty == 0 )
		PANIC("iget: no free slot in icache!");

	ip = empty;
	ip->dev = dev;
	ip->inum = inum;
	ip->ref = 1;
	kprintf("iget: loc of ip: %h", ip);
	return ip;
}


int readi(struct inode *ip, char *dst, uint32_t off, uint32_t n)
{
	struct dirent de;
	char data[512];
	char *p;
	kprintf("readi: &data: %h", &data);
	kprintf("readi: ip: %h", ip);
	if (ip->dev == 1) {
		p = readinitrd(ip, off, n);
		kprintf("readi:XXXXXXXXXXXXXXXXXXXXXXXXXXXXX p: %h", p);
		//safestrcpy(data, p, n);
	}
	memmove(dst, p, n);
	return n;
}
