// User space program

#include "user.h"


void main()
{
	printf(0, "something big\0");

	for(;;)
		asm("hlt");
}
