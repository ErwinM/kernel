// User space program: sh shell

void main()
{
	char *buf[64];
	int n;

	printf(0,"ErwinOS shell loaded.\n");
	printf(0,"ErwinOS:/ $ ");

	n = read(0, buf, 100);
	printf(0, "got: %s", buf);
}
