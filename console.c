#include "defs.h"
#include "common.h"
#include "kbd.h"

static struct {
  struct spinlock lock;
  int locking;
} cons;

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
	switch(c){
	case 'p':
		procdump();
		break;
	case 255:
		return;
	case '\n':
		clist.buf[clist.wi++] = '\n';
		kprintf("WAKEUP",0);
		wakeup(&clist.wi);
		break;
	default:
		fb_put_char(c);
	}
	if(clist.wi == 128)
		PANIC("consoleint: buffer is full!");
	if(c == '\b'){
		clist.buf[clist.wi--] = 0;
	} else {
		clist.buf[clist.wi++] = c;
	}
}

int consoleread(struct inode *ip, char *dst, uint32_t n)
{
	int len;

	acquire(&clist.lock);
	while(n > 0){
		while(clist.ri == clist.wi){
			sleep(&clist.wi, &clist.lock); // wait for something to arrive in buffer
		}
		for(clist.ri = 0; clist.ri<=clist.wi;){
			*dst++ = clist.buf[clist.ri++];
		}
		n = 0; // enter always ends the console read
	}
	*dst++ = 0;
	len = clist.ri;
	clist.ri = clist.wi = 0;
	return len+1;
}

static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint32_t x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    fb_put_char(buf[i]);
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void
kprintf(char *fmt, ...)
{
  int i, c, locking;
  uint32_t *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    PANIC("null fmt");

  argp = (uint32_t*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      fb_put_char(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
		case 'h':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        fb_put_char(*s);
      break;
    case '%':
      fb_put_char('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      fb_put_char('%');
      fb_put_char(c);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}
