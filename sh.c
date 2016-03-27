// User space program: sh shell

#include "utypes.h"

#define MAXARGS 10

char *title = "ErwinOS shell loaded.\n";

void main()
{
	char *line[64], *cmd, *argv[MAXARGS];
	int n, i;


	cmd = malloc(64);

	printf(0,"sh: malloc returned: %x", cmd);
	//argv = malloc(128);

	//printf(0, "cmd loc: %d", (unsigned int)cmd);

	printf(0,"%s", title);
	for(;;){  // endless loop
		printf(0,"\nErwinOS:/ $ ");

		n = read(0, line, 64);
		//printf(0,"sh: line>>>>>>%s<<<<<<", line);
		n = splitline(line, cmd, n);
		printf(0, "Executing: %s", cmd);

		argv[0] = cmd;
		argv[1] = 0;

		if(fork() == 0) // child process
			exec(cmd, argv);
		waitt(); // parent waits for child to exit
	}
}

int splitline(char *line, char *cmd, int len)
{
	char *s, *space;
	int k, i;// User space program: sh shell

#include "utypes.h"

char *title = "ErwinOS shell loaded.\n";

void main()
{
	char *line[64], *cmd, *argv;
	int n;


	cmd = malloc(64);

	printf(0,"sh: malloc returned: %x", cmd);
	//argv = malloc(128);
// User space program: sh shell

#include "utypes.h"

char *title = "ErwinOS shell loaded.\n";

void main()
{
	char *line[64], *cmd, *argv;
	int n;


	cmd = malloc(64);

	printf(0,"sh: malloc returned: %x", cmd);
	//argv = malloc(128);

	//printf(0, "cmd loc: %d", (unsigned int)cmd);

	printf(0,"%s", title);
	printf(0,"ErwinOS:/ $ ");

	n = read(0, line, 64);
	//printf(0,"sh: line>>>>>>%s<<<<<<", line);
	n = splitline(line, cmd, n);
	printf(0, "Executing: %s", cmd);
	if(fork() == 0) // child process
		exec(cmd, 0);
	waitt(); // parent waits for child to exit
}

int splitline(char *line, char *cmd, int len)
{
	char *s, *space;
	int k, i;
	if((space = strchr(line, 0x20)) == 0){
		i = strlen(line);
	} else {
		// printf(0,"\nparseline: line ptr: %x and cmd pitr: %x and fspace: %p", line, cmd, space);
		i = (uint32_t)space - (uint32_t)line;
		//printf(0, "\nparseline: i: %d", i);
	}
	memmove(cmd, line, i);
	printf(0,"parseline: strchr returned");

	return len;
}


// NEED MALLOC! GOT IT!

	//printf(0, "cmd loc: %d", (unsigned int)cmd);

	printf(0,"%s", title);
	printf(0,"ErwinOS:/ $ ");

	n = read(0, line, 64);
	//printf(0,"sh: line>>>>>>%s<<<<<<", line);
	n = splitline(line, cmd, n);
	printf(0, "Executing: %s", cmd);
	if(fork() == 0) // child process
		exec(cmd, 0);
	waitt(); // parent waits for child to exit
}

int splitline(char *line, char *cmd, int len)
{
	char *s, *space;
	int k, i;
	if((space = strchr(line, 0x20)) == 0){
		i = strlen(line);
	} else {
		// printf(0,"\nparseline: line ptr: %x and cmd pitr: %x and fspace: %p", line, cmd, space);
		i = (uint32_t)space - (uint32_t)line;
		//printf(0, "\nparseline: i: %d", i);
	}
	memmove(cmd, line, i);
	printf(0,"parseline: strchr returned");

	return len;
}


// NEED MALLOC! GOT IT!

	if((space = strchr(line, 0x20)) == 0){
		i = strlen(line);
	} else {
		// printf(0,"\nparseline: line ptr: %x and cmd pitr: %x and fspace: %p", line, cmd, space);
		i = (uint32_t)space - (uint32_t)line;
		//printf(0, "\nparseline: i: %d", i);
	}
	memmove(cmd, line, i);
	printf(0,"parseline: strchr returned");

	return len;
}


// NEED MALLOC! GOT IT!
