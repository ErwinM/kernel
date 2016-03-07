#include "defs.h"
#include "common.h"



struct {
	struct spinlock lock;
	char buf[128];
	uint32_t ri; // read index
	uint32_t wi; // write index
} clist;

void consoleint(void)
{
	uint8_t c;

	// get the key and add it to the buffer
	c = kbdgetc();
	if(c == 255)
		return;
	fb_put_char(c);
	if(c == '\n'){
		kprintf("WAKEUP",0);
		wakeup(&clist.wi);
	}
	if(clist.wi == 128)
		PANIC("consoleint: buffer is full!");
	clist.buf[clist.ri++] = c;
}


int consoleread(struct inode *ip, char *dst, uint32_t n)
{
	int len;

	acquire(&clist.lock);
	while(n > 0){
		while(clist.ri == clist.wi){
			sleep(&clist.wi, &clist.lock); // wait for something to arrive in buffer
		}
		for(clist.wi = 0; clist.wi<=clist.ri;){
			*dst++ = clist.buf[clist.wi++];
		}
		n = 0; // enter always ends the console read
	}
	len = clist.ri;
	clist.ri = clist.wi = 0;
	return len;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
extern unsigned short *vidptr;

void
cgaputc(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if(c == BACKSPACE){
    if(pos > 0) --pos;
  } else
    vidptr[pos++] = (c&0xff) | 0x0700;  // black on white

  if((pos/80) >= 24){  // Scroll up.
    memmove(vidptr, vidptr+80, sizeof(vidptr[0])*23*80);
    pos -= 80;
    memset(vidptr+pos, 0, sizeof(vidptr[0])*(24*80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  vidptr[pos] = ' ' | 0x0700;
}
