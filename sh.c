// User space program: sh shell

	char *title = "ErwinOS shell loaded.\n";

void main()
{
	char *cmd[64];
	int n;



	printf(0,"%s", title);
	printf(0,"ErwinOS:/ $ ");

	n = read(0, cmd, 64);
	printf(0, "got: %s", cmd);
	printf(0, "len: %d", n);
}

void parseline(char *line)
{
	char s;
	s = strchr(line);


}


// NEED MALLOC!
