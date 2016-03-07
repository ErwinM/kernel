#include "common.h"
#include "defs.h"
#include "kbd.h"

uint8_t kbdgetc(void)
{
	uint8_t sc;
	uint8_t c;

	sc = inb(KBDATAP);
	if((sc & (1 << 7)) == 0) {
		c = normalmap[sc];
		return c;
	} else {
		return 255;
	}
}
