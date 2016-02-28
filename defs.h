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
void 				memcpy(void*, void*, uint32_t);
void 				memset(void*, int, uint32_t);
void* 			memmove(void*, const void*, uint32_t);
char* 			safestrcpy(char*, const char*, int);
// fs.c
int 				readi(struct inode*, char*, uint32_t, uint32_t);
// initrd.c
void 				iinitrd(uint32_t);
char* 			readinitrd(struct inode*, uint32_t, uint32_t);
int * 			listinitrd(struct inode*, struct dirent*);

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
void 				fb_printf(char *buf, uint32_t n);
void 				kprintf(char *buf, uint32_t n);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))

#endif
