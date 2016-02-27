// fs driver for the initial ramdsk
// single directory fs
// Inodes are not on disk as inodes so have to be constructed here...

#include "common.h"
#include "initrd.h"
#include "file.h"
#include "fs.h"

extern uint32_t *initrd;
extern struct devsw devsw[];

struct inode *root;
struct initrdsb *sb;
struct initrdhdr *initrdhdrs;
struct dirent de[10];

void iinitrd(uint32_t *location)
{
	sb = location;
	initrdhdrs = (struct initrdhdr*)((uint32_t)location + sizeof(struct initrdsb));
	//devsw[D_INITDR].read = readinitrd;
	kprintf("initrd: sb: %h", sb->nfiles);
	kprintf("iinitrd: header[0]:",0);
	fb_write((*initrdhdrs).name);
	uint32_t loc = (uint32_t)location + (uint32_t)(*initrdhdrs).offset;
	kprintf("iinitrd offset: %h", loc);
}

char * readinitrd(struct inode *ip, uint32_t off, uint32_t n)
{
	struct initrdbuf *buf;
	uint32_t dummy1 = off;
	uint32_t dummy2 = n;
	kprintf("readinitrd: ip: %h", ip);
	if(ip->inum == 1){
		//we should return a bunch of dirent entries
		// how many?
		int k=1;
		for(k = 0; k < sb->nfiles; k++ ){
			safestrcpy(&de[0].inum,k); // TO DO
			safestrcpy(&de[0].name, (initrdhdrs[0]).name);
		}
		return &de;
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
