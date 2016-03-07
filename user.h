

// syscalls

int 				fork(void);


// user lib functions
void printf(int, char*);



#define PANIC(msg) panic(msg, __FILE__, __LINE__);

extern void panic(const char *message, const char *file, int line);
