// implementation of the vfs that deal with i-nodes
#include "defs.h"
#include "common.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "file.h"
#include "initrd.h"

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
	//kprintf("iget: icache loc: %h", &icache);
	// check if already in inode cache, if so fetch it
	struct inode *ip, *empty;
	empty = 0;
	for (ip = &icache.inode[0] ; ip < &icache.inode[NINODE] ; ip++){
		if (ip->ref > 0 && ip->dev == dev && ip->inum == inum ) {
			ip->ref++;
			return ip;
		}
		if (empty == 0 && ip->ref == 0 ){
			// remember empty slot
			empty = ip;
			//kprintf("iget: empty: %h", empty);
		}
	}
	// if not allocate a cache entry
	if (empty == 0 )
		PANIC("iget: no free slot in icache!");

	ip = empty;
	ip->dev = dev;
	ip->inum = inum;
	ip->ref = 1;
	//kprintf("iget: loc of ip: %h", ip);
	return ip;
}


int readi(struct inode *ip, char *dst, uint32_t off, uint32_t n)
{
	struct rdbuf *b;

	if (ip->dev == 1) {
		b = readinitrd(ip, off, n);
	}
	memmove(dst, b->data, n);
	//rdrelse(b);
	return n;
}

struct inode* dirlookup(struct inode *dp, char *name)
{

}

// returns the inode for the given path
struct inode* namei(char *path)
{
	// initially get it working for the initrd (no subdirs)
	char *name[DIRSIZ];
	char *s;
	int len;
	struct inode *root;

	while (*path == '/')
		path++;
	s = path;

	while (*path != '/' && *path != 0)
		path++;
	len = path - s;

	memcpy(name, s, len);
	name[len] = 0;

	root = iget(1,1);



	//search the current wd for the namei

}
