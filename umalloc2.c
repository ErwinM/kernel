#include "common.h"

struct hdr {
	uint32_t sz;
	uint32_t used;
	uint32_t magic;
	uint32_t *nfree;
};


struct hdr *baseptr, *freeptr;


#define align4(x) (((((x)-1)>>2)<<2)+4)
#define MAGIC  0xabcdef

void *expheap(uint32_t sz)
{
	uint32_t s;
	struct hdr *bptr;

	s = sz + 2 * sizeof(struct hdr);

	if((bptr = sbrk(s)) < 0)
		PANIC("exphead: sbrk failed");

	return bptr;
}

void *malloc(uint32_t n)
{
	uint32_t s, i, ss;
	struct hdr *hp, *naddr;

	s = align4(n);
	printf(0, "malloc: request for: %d", n);
	if(!baseptr) {
		printf(0, "\nmalloc: init heap!");
		// no user heap yet
		baseptr = expheap(s);
		baseptr->sz = s;
		baseptr->used = 0;
		baseptr->nfree = 0;
		baseptr->magic = MAGIC;
		freeptr = baseptr;
	}
	printf(0, "\nmalloc: freeptr: %x", freeptr);
	//if(freeptr == 0){
		//printf(0,"\nrustig");
	hp = freeptr;
	printf(0, "\nmalloc: hp->sz: %d", hp->sz);
	while(hp->sz < s){
		printf(0, "\nmalloc: block is too small");
		hp = hp->nfree;
		if(hp == freeptr){
			//full circle without hit
			// expand uheap
		}
	}
	printf(0, "\nmalloc: hp: %x", hp);
	if((hp->sz - s) <= sizeof(struct hdr)){
		s = hp->sz;
		hp->used = 1;
		hp->nfree = 0;
		freeptr = hp->nfree;
		hp++;
		return hp;
	} else {
		naddr = (uint32_t*)(uint32_t)hp + s;
		naddr->sz = hp->sz - s - sizeof(struct hdr);
		naddr->used = 0;
		naddr->nfree = hp->nfree;
		freeptr = hp->nfree;
		return (uint32_t*)(uint32_t)naddr + sizeof(struct hdr);
	}
}
