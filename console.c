#include "defs.h"
#include "common.h"



struct {
	struct spinlock lock;
	char buf[128];
	uint32_t ri; // read index
	uint32_t wi; // write index
} clist;

void consoleintr(void)
{


}

int consoleread(struct inode *ip, char *dst, uint32_t n)
{
	acquire(&clist.lock);
	while(n > 0){
		while(clist.ri == clist.wi){
			sleep(&clist.ri, &clist.lock); // wait for something to arrive in buffer
		}
	}
}
