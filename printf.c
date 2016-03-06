// User space program

void putc(int fd, char c)
{
	write(fd, &c, 1);
}

void printf(int fd, char *fmt)
{
	int i;
	char c;

	for(i = 0; fmt[i]; i++){
		c = fmt[i] & 0xff;
		putc(fd, c);
	}
}
