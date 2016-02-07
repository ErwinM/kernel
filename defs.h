#ifndef INCLUDE_DEFS_H
#define INCLUDE_DEFS_H

#include "common.h"
#include "proc.h"

// alloc.c
void 				initkheap();
void 				kfree(uint32_t *address);
uint32_t* 	kalloc();

// common.h
void 				memcpy(uint16_t *dest, uint16_t *src, uint32_t len);
void 				memset(uint8_t *dest, uint8_t val, uint32_t len);

void* 			memmove(void *dst, const void *src, uint32_t n);
char* 			safestrcpy(char *s, const char *t, int n);

// vm.c
void 				initgtd();
void 				initpaging();
void 				inituvm(pte_t *pgdir, char *init, uint32_t sz);
pte_t* 			setupkvm();
void switchuvm(struct proc *p);

#endif
