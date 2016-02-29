// fs driver for the initial ramdsk
// single directory fs
// Inodes are not on disk as inodes so have to be constructed here...

#include "common.h"
#include "initrd.h"
#include "file.h"
#include "fs.h"
//#include "defs.h"

extern uint32_t *initrd;
extern struct devsw devsw[];

struct inode *root;
struct initrdsb *sb;
struct initrdhdr *initrdhdrs;

struct rdbuf rdbuf[32];


void iinitrd(uint32_t *location)
{
	struct inode *root;

	sb = location;
	initrdhdrs = (struct initrdhdr*)((uint32_t)location + sizeof(struct initrdsb));
	//devsw[D_INITDR].read = readinitrd;
	kprintf("initrd: sb: %h", sb->nfiles);
	uint32_t loc = (uint32_t)location + (uint32_t)(*initrdhdrs).offset;
	kprintf("iinitrd offset: %h", loc);

	root = iget(1,1);
	root->size = sb->nfiles;

}

struct rdbuf* rdget()
{
	struct rdbuf *b;

	b = &rdbuf;
	while ( b->flags == B_BUSY && b < &rdbuf[32] )
		b++;
	if (b->flags == B_BUSY)
		PANIC("rdget: No free buffers!");
	b->flags == B_BUSY;
	return b;
}

char *readinitrd(struct inode *ip, uint32_t off, uint32_t n)
{
	struct dirent *de;
	struct rdbuf *b;
	uint32_t nr;

	b = rdget();

	if(ip->inum == 1){
		// Reading root: (e.g. searching)
		// We should return the nth dirent entry...
		nr = off / sizeof(struct dirent);
		if (off % sizeof(struct dirent) !=0 || nr > (ip->size - 1)) {
			PANIC("readinitrd: out of bounds offset");
		}
		nr = off / sizeof(struct dirent);
		de = b->data;
		de->inum = nr + 2; // 1 = root
		safestrcpy(de->name, (initrdhdrs[nr]).name);
		//memcpy(&initrdbuf, data, n);
		b->data[n] = 0;
		return b;
	}
	PANIC("readinitrd: inode != root!");
}

int * listinitrd(struct inode *ip, struct dirent *dst)
{
	if (!ip->inum == 1 )
		PANIC("listinitrd: request dir listing from other than root");

	int k;
	//kprintf("listinitrd: sb->nfiles: %d", sb->nfiles);
	for (k = 0 ; k < sb->nfiles ; k++ ){
		safestrcpy(dst[k].inum, k );
		safestrcpy(dst[k].name, initrdhdrs[k].name);
		//fb_write("listinitrd: header name: ");
		//fb_write(initrdhdrs[k].name);
	}
	return sb->nfiles;
}
