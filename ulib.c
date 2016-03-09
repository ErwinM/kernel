


extern void panic(const char *message, const char *file, int line)
{
    // We encountered a massive problem and have to stop.
    asm volatile("cli"); // Disable interrupts.
    printf("PANIC(");
    printf(message);
    printf(") at ");
    printf(file);
    printf(":");
    printf(line);
    printf("\n");
    // trigger Bochs debug mode (magic instruction)
		asm volatile("xchg %bx, %bx");
}

char *strchr(char *str, char c)
{
	char *s;

	for(s = str; s < strlen(str); s++)
		if(*s = c)
			break;
	return s;
}

int strlen(char *s)
{
  int n;
  for ( n = 0 ; s[n]>0 ; n++ )
    ;
  return n;
}

char* gets(char *buf, int max)
{
	int n, i;

	n = read(0, buf, max);
	for(i = 0; i <=n; i++)
		if(buf[i] = '\n')
			break;
	buf[i] = '\0';
	return buf;
}
