#ifndef INCLUDE_VM_H
#define INCLUDE_VM_H

#include "paging.h"

page_dir_t* setupkvm();
void switchuvm(struct proc *p);
void inituvm(pde_t *pgdir, char *init, uint32_t sz);

#endif
