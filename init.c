// User space program
#include "user.h"
#include "common.h"

void main()
{
	int pid;
	char *argv[] = { "sh", 0 };
	printf(0,"\nInit process starting sh...");


	pid = fork();

	if(pid < 0) {
		PANIC("init: fork call failed");
	}

	if(pid == 0) {
		printf(0,"init: pid: %d", pid);
		exec("sh", argv);
		PANIC("init: exec call failed");
	}
	int i;
	for(;;)
		i++;
	printf(0,"Init: FINISHED!");
}
