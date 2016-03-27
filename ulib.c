


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

void* memmove(void *dst, const void *src, unsigned int n)
{
  const char *s;
  char *d;

  s = src;
  d = dst;
  if(s < d && s + n > d){
    s += n;
    d += n;
    while(n-- > 0)
      *--d = *--s;
  } else
    while(n-- > 0)
      *d++ = *s++;

  return dst;
}

char* strchr(const char *s, char c)
{
	printf(0, "strchr: s is %s", s);
  for(; *s; s++)
    if(*s == c) {
      return (char*)s;
		}
  return 0;
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
	printf(0,"gets: read returned: %d", n);
	for(i = 0; i <=n; i++)
		if(buf[i] = '\n')
			break;
	buf[i] = '\0';
	return buf;
}
