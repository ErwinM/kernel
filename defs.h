#ifndef INCLUDE_DEFS_H
#define INCLUDE_DEFS_H

#include "common.h"
#include "proc.h"
//#include "file.h"

// alloc.c
void 				initkheap();
void 				kfree(uint32_t *address);
uint32_t* 	kalloc();
uint32_t*		kmalloc(uint32_t, int);
uint32_t*		kmalloc_a(uint32_t);
// common.h
void*				memcpy(void*, const void*, uint32_t);
void 				memset(void*, int, uint32_t);
void* 			memmove(void*, const void*, uint32_t);
char* 			safestrcpy(char*, const char*, int);
int 				strlen(char*);
int					strncmp(const char*, const char*, uint32_t);
// fs.c
int 				readi(struct inode*, char*, uint32_t, uint32_t);
// initrd.c
void 				iinitrd(uint32_t);
char* 			readinitrd(struct inode*, uint32_t, uint32_t);
int * 			listinitrd(struct inode*, struct dirent*);
int 				rdrelse(struct rdbuf*);
// proc.c
int					fork(void);


// syscall.c
int 				argint(uint32_t, uint32_t*);
int					argptr(int, char**, int);
int					argstr(int, char**);

// vm.c
void 				initgtd();
void 				initpaging();
void 				inituvm(pte_t *pgdir, char *init, uint32_t sz);
pte_t* 			setupkvm();
void 				switchuvm(struct proc *p);

// write.c
void 				fb_clear(void);
void 				fb_write(char *buf);
void 				fb_init(uint8_t n);
void  			kprintf(char *fmt, ...);
void 				fb_put_char(char);

// x86.h
void 				bbrk(void);
	/* code */

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))

#endif
