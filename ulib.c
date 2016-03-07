


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
