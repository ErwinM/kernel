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

extern struct proc *cp;

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
	//memset(ip->addrs, 0, sizeof(uint32_t[NDIRECT+1]));
	//kprintf("iget: loc of ip: %h", ip);
	return ip;
}

// read the inode from disk
void ilock(struct inode *ip)
{
	getinitrd(ip);
}

int readi(struct inode *ip, char *dst, uint32_t off, uint32_t n)
{
	struct rdbuf *b;

	if (ip->dev == 1) {
		b = readinitrd(ip, off, n);
	}
	memmove(dst, b->data, n);
	rdrelse(b);
	return n;
}

struct inode* dirlookup(struct inode *dp, char *name)
{
	struct dirent de;
	int off, inum;

	if (dp->type != T_DIR)
		PANIC("dirlookup: inode not a dir");

	kprintf("dirlookup: sizeof de: %d", sizeof(de));
	for(off = 0; off <= dp->size ; off +=sizeof(de) ) {
		fb_write("testing");
		if(readi(dp, &de, off, sizeof(de)) == -1)
			PANIC("dirlookup: readi failed!");
		if(strncmp(de.name, name, DIRSIZ) == 0) {
			// match
			inum = de.inum;
			return iget(dp->dev, inum);
		}
	}
	PANIC("dirlookup: name not found");
}

// commits an in memory inode to disk
void iupdate(struct inode *ip)
{
	updateinitrd(ip);
}

// returns the inode for the given path
struct inode* namei(char *path)
{
	// initially get it working for the initrd (no subdirs)
	char *name[DIRSIZ];
	char *s;
	int len;
	struct inode *dp, *ip;
	if (1==1) // curently, only 1 dir so always root
		dp = iget(ROOTDEV, ROOTINO);
	else
		dp = cp->cwd;
	while (*path == '/')
		path++;
	s = path;
	while (*path != '/' && *path != 0)
		path++;
	len = path - s;
	memcpy(name, s, len);
	name[len] = 0;
	kprintf("namei: looking for >>>",0);
	fb_write(name);
	fb_write("<<<<");
	if((ip = dirlookup(dp, name)) == 0)
		PANIC("namei: cannot find file");
	return ip;
}
