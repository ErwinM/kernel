#ifndef INCLUDE_VM_H
#define INCLUDE_VM_H

#include "paging.h"

pte_t* setupkvm();
void switchuvm(struct proc *p);
void inituvm(pte_t *pgdir, char *init, uint32_t sz);

#endif
