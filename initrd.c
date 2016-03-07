// fs driver for the initial ramdsk
// single directory fs: file info is integrated into the header files
// Headers need to be translated to inodes

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
		kprintf("iinitrd offset: %h", location);
	kprintf("iinitrd offset: %h", initrdhdrs);
	kprintf("iinitrd offset: %h", loc);

	root = iget(1,1);
	root->size = (uint32_t)sizeof(struct dirent) * sb->nfiles;
	root->type = T_DIR;

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
	memset(b->data, 0, 512);
	return b;
}

int rdrelse(struct rdbuf *b)
{
	b->flags = 0;
	return 0;
}

struct initrdhdr *getinitrdhdr(struct inode *ip)
{
	// get the header that corresponds to the inode
	struct initrdhdr *hdr;

	hdr = initrdhdrs + (ip->inum - 2);
	if(hdr->magic != 0xBF)
		PANIC("updateinitrd: cannot find header!");
	return hdr;
}

void getinitrd(struct inode *ip)
{
	struct initrdhdr *hdr;

	hdr = getinitrdhdr(ip);
	ip->size = hdr->sz;
	ip->addrs[0] = hdr->offset;
	//kprintf("getinitrd: hdr->offset: %h",hdr->offset);
	//kprintf("getinitrd: ip->addrs: %h",*ip->addrs);
}

struct rdbuf *readinitrd(struct inode *ip, uint32_t off, uint32_t n)
{
	struct dirent *de;
	struct rdbuf *b;
	uint32_t nr;
	char *offset;

	b = rdget();

	if(ip->inum == 1){
		// Reading root: (e.g. searching)
		// We should return the nth dirent entry...
		nr = off / sizeof(struct dirent);
		if (off % sizeof(struct dirent) !=0 || off > ip->size) {
			PANIC("readinitrd: out of bounds offset");
		}
		nr = off / sizeof(struct dirent);
		de = b->data;
		de->inum = nr + 2; // 1 = root
		safestrcpy(de->name, (initrdhdrs[nr]).name);
		b->data[n] = 0;
		return b;
	} else {
		// Reading a file
		if((off + n) > ip->size)
			PANIC("readinitrd: reading past end of file.");
		offset = (uint32_t)sb + ip->addrs[0] + off;
		//kprintf("readinitrd: offset: %h", n);
		memmove(b->data, offset, n);
		//b->data[n+1] = 0;
		//kprintf("readinitrd: data>>>",0);
		//fb_write(b->data);
		//fb_write("<<<<<");
		return b;
	}
}

void updateinitrd(struct inode *ip)
{
	struct initrdhdr *hdr;
	hdr = getinitrdhdr(ip);
	hdr->sz = ip->size;
	hdr->magic = 0xBF;
	hdr->offset = ip->addrs;
}
