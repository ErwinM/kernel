#include "param.h"
#include "common.h"
#include "paging.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

extern struct proc *proc;
extern struct cpu *mcpu;
struct segdesc gdt[NSEGS];


void initgdt()
{

		fb_write("Setting up Global Descriptor Table...");
		mcpu->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
		mcpu->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
		mcpu->gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
		mcpu->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
		lgdt(mcpu->gdt, sizeof(mcpu->gdt));
		fb_write("Succes.\n");
}

// Sets up a new pgdir and maps the kernel part in
page_dir_t* setupkvm()
{
	// setup a new pgdir
	page_dir_t *pgdir;
	int k;
	if((pgdir = (page_dir_t*)kalloc()) == 0 )
		PANIC("setupkvm: out of kheap memory.");
	memset(pgdir, 0, PGSIZE);
	// map 0..1mb to 0xc00000
	for (k = 0 ; (k + PGSIZE) <= 0x100000 ; k += 0x1000){
		mappage(pgdir, k, (0xc00000 + k), PTE_W);
	}
	// identity map 0xd00000..0xffffff
	for (k = 0xd00000 ; (k + PGSIZE) <= 0x1000000 ; k += 0x1000){
		mappage(pgdir, k, k, PTE_W );
	}
	return pgdir;
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
void switchkvm(void)
{
  lcr3(proc);   // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
void switchuvm(struct proc *p)
{
  pushcli();
  mcpu->gdt[SEG_TSS] = SEG16(STS_T32A, &mcpu->ts, sizeof(mcpu->ts)-1, 0);
  mcpu->gdt[SEG_TSS].s = 0;
  mcpu->ts.ss0 = SEG_KDATA << 3;
  mcpu->ts.esp0 = (uint32_t)proc->kstack + KSTACK_SIZE;
  ltr(SEG_TSS << 3);
  if(p->pgdir == 0)
    PANIC("switchuvm: no pgdir");
  lcr3(p->pgdir);  // switch to new address space
  popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
// THIS ALLOCATION MECHANISM NEEDS TO CHANGE LATER
// IT IS NOW ALLOCATING IN KERNEL MEMORY
void inituvm(pde_t *pgdir, char *init, uint32_t sz)
{
  char *mem;

  if(sz >= PGSIZE)
    PANIC("inituvm: more than a page");
  mem = kalloc();
  memset(mem, 0, PGSIZE);
  mappage(pgdir, mem, 0, PTE_W|PTE_U);
  memmove(mem, init, sz);
}
