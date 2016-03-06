#include "defs.h"
#include "x86.h"
#include "param.h"
#include "common.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

extern struct proc *cp;
extern struct cpu *mcpu;
extern struct cpu maincpu;

struct segdesc gdt[NSEGS];
pde_t *kpgdir;


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

pageinfo mm_virtaddrtopageindex(uint32_t virtaddr){
    pageinfo pginf;
    //align address to 4k (highest 20-bits of address)
    virtaddr &= ~0xFFF;
    pginf.pagetable = virtaddr / 0x400000; // each page table covers 0x400000 bytes in memory
    pginf.page = (virtaddr % 0x400000) / 0x1000; //0x1000 = page size
    return pginf;
}

void initpaging()
{
	fb_write("Initialising paging...");
	kpgdir = kmalloc_a(PGSIZE);
	memset(kpgdir, 0, PGSIZE);
	int k;
	for (k = 0; k <= 1; k++){
		pte_t *pgtable;
		pgtable = kmalloc_a(PGSIZE);
		memset(pgtable, 0, PGSIZE);
		kpgdir[k] = (uint32_t)pgtable + 3;
	}
	for (k = 0; (k + PGSIZE) <= 0x100000 ; k += 0x1000){
		mappage(kpgdir, k, k, PTE_W);
		mappage(kpgdir, k, (0x400000 + k), PTE_W);
	}
	for (k = 0x500000; (k + PGSIZE) <= 0x800000 ; k += 0x1000){
		mappage(kpgdir, k, k, PTE_W);
	}

  // Now, enable paging!
	//fb_printf("Enabling paging by loading CR3 with: %h", kpgdir);
  lcr3(kpgdir);
	enablepag();
	fb_init(1); // Redirect pointer to video memory
	fb_write("..enabled.\n");
}

// Return the physical address of the PAGE in directory pgdir
// that corresponds to virtual address va.
uint32_t* walkpagedir(pte_t *pgdir, uint32_t vaddr)
{
	uint32_t *pgtable;
	pageinfo pginf = mm_virtaddrtopageindex(vaddr);
	pgtable = (uint32_t*)((uint32_t)pgdir[pginf.pagetable] & 0xFFFFF000);
	kprintf("walkpagedir: page: %h", pgtable[pginf.page]);
	return pgtable[pginf.page];
}

void mappage(uint32_t *pgdir, uint32_t paddr, uint32_t vaddr, int perm)
{
	//kprintf("MAPPAGE: mapping phys: %h", paddr);
	//kprintf(" to virt: %h.\n", vaddr);
	//kprintf("MAPPAGE: pgdir: %h", pgdir);
	pageinfo pginf = mm_virtaddrtopageindex(vaddr);
	uint32_t *pgtable;
	if (pgdir[pginf.pagetable] & 1){
		// pgtable exisits
		pgtable = (pte_t*)(pgdir[pginf.pagetable] & 0xFFFFF000);
		if(pgtable[pginf.page] & 1){
			kprintf("vaddr: %h", vaddr);
			PANIC("mappage: vaddr already mapped!");
		}
	} else {
		// pgtable does not exist, make it
		// fb_write("PT does not exist!");
		if((pgtable = kalloc()) == 0){
			PANIC("mappage: kalloc returned 0");
		}
		pgdir[pginf.pagetable] = (uint32_t)pgtable | perm | PTE_P;
		memset(pgtable, 0, PGSIZE);
	}
	pgtable[pginf.page] = paddr | perm | PTE_P;
}

// Sets up a new pgdir and maps the kernel part in
pte_t* setupkvm()
{
	// setup a new pgdir
	pte_t *pgdir;
	if((pgdir = (pte_t*)kalloc()) == 0 )
		PANIC("setupkvm: out of kheap memory.");
	memset(pgdir, 0, PGSIZE);
	// map 0..1mb to 0xc00000
	int k;
	for (k = 0 ; (k + PGSIZE) <= 0x100000 ; k += 0x1000){
		mappage(pgdir, k, (FIRSTMB + k), PTE_W);
	}
	// identity map 0xd00000..0xffffff
	for (k = KCRITICAL ; (k + PGSIZE) <= 0x1000000 ; k += 0x1000){
		mappage(pgdir, k, k, PTE_W );
	}
	return pgdir;
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
void switchkvm(void)
{
  lcr3(kpgdir);   // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
void switchuvm(struct proc *p)
{
  pushcli();
  mcpu->gdt[SEG_TSS] = SEG16(STS_T32A, &mcpu->ts, sizeof(mcpu->ts)-1, 0);
  mcpu->gdt[SEG_TSS].s = 0;
  mcpu->ts.ss0 = SEG_KDATA << 3;
  mcpu->ts.esp0 = (uint32_t)cp->kstack + KSTACK_SIZE;
	kprintf("switchuvm: esp: %h\n", ((uint32_t)cp->kstack + KSTACK_SIZE));
	ltr(SEG_TSS << 3);
  if(p->pgdir == 0)
    PANIC("switchuvm: no pgdir");
	kprintf("switchuvm: loading lcr3 with: %h..", p->pgdir);
  lcr3(p->pgdir);  // switch to new address space
	kprintf("..loaded.",0);
  popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
void inituvm(pte_t *pgdir, char *init, uint32_t sz)
{
  char *mem;

  if(sz >= PGSIZE)
    PANIC("inituvm: more than a page");
  mem = kalloc();
	kprintf("inituvm: got location: %h", init);
  memset(mem, 0, PGSIZE);
  mappage(pgdir, mem, 0, PTE_W|PTE_U);
  memmove(mem, init, sz);
}

pde_t copyuvm(uint32_t *pgdir, uint32_t sz)
{
	pde_t *dir;
	pte_t *pte;
	uint32_t k;
	char *mem;

	if((dir = setupkvm()) == 0)
		PANIC("copyuvm: error on setupkvm");

	for(k = 0; k < sz; k += PGSIZE){
		// error checking
		if((pte = walkpagedir(pgdir, k)) == 0)
			PANIC("copyuvm: no entry for pte");
		if(!(*pte & PTE_P))
			PANIC("copyuvm: page not present");
		// copy the pages themselves
		if((mem = kalloc()) == 0)
			PANIC("copyuvm: kalloc failed");
		memmove(mem, (char*)*pte, PGSIZE);
		// map them in the new pgdir
		mappage(dir, mem, PTE_ADDR(*pte), PTE_FLAGS(*pte));
	}
	return dir;
}

// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
int loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint32_t phoffset, uint32_t sz)
{
	uint32_t va, n;
	pte_t *pte;
	char *pa;

	if((uint32_t)addr%PGSIZE != 0)
		PANIC("loaduvm: addr not page aligned");
	for(va = 0; va < sz; va += PGSIZE){
		pte = walkpagedir(pgdir, va);
		pa = PTE_ADDR(pte);
		kprintf("loaduvm: pa: %h", pa);
		if(sz-va < PGSIZE)
			n = sz -va;
		else
			n = PGSIZE;
		if(readi(ip, pa, phoffset + va, n) < n)
			PANIC("loaduvm: readi");
	}
	return 0;
}

pte_t allocuvm(pde_t *pgdir, uint32_t oldsz, uint32_t newsz)
{
	uint32_t k;
	char *mem;

	k = PGROUNDUP(oldsz);
	for(; k < newsz; k+= PGSIZE){
		mem = kalloc();
		memset(mem, 0, PGSIZE);
		mappage(pgdir, mem, (char*)k, PTE_U|PTE_W);
		kprintf("allocuvm: mapped: %h", mem);
	}
	return newsz;
}

int copyout(pde_t *pgdir, uint32_t va, void *p, uint32_t len)
{
	pte_t *pte;
	uint32_t pa, va0, pa0, n;
	char *buf;

	buf = (char*)p;
	while ( len > 0) {
		va0 = PGROUNDDOWN(va);
		pte = walkpagedir(pgdir, va0);
		kprintf("copyout: pte: %h", pte);
		if(((uint32_t)pte & PTE_P) == 0 || ((uint32_t)pte & PTE_U) == 0 )
			PANIC("copyout: page not usable!");
		pa0 = PTE_ADDR(pte);
		if(pa0 == 0)
			PANIC("copyout: invalid physical address");
		n = PGSIZE - (va - va0);
		if (n > len)
			n = len;
		memmove(pa0 + (va - va0), buf, n);
		len -= n;
		buf += n;
		va = va0 + PGSIZE;
	}
}

void page_fault(uint32_t err)
{
   // A page fault has occurred.
   // The faulting address is stored in the CR2 register.
   uint32_t faulting_address;
   asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

   // The error code gives us details of what happened.
   int present   = !(err & 0x1); // Page not present
   int rw = err & 0x2;           // Write operation?
   int us = err & 0x4;           // Processor was in user-mode?
   int reserved = err & 0x8;     // Overwritten CPU-reserved bits of page entry?
   int id = err & 0x10;          // Caused by an instruction fetch?

	 kprintf("Err: %h", err);
   // Output an error message.
   fb_write("Page fault! ( ");
   if (present) {fb_write("present ");}
   if (rw) {fb_write("read-only ");}
   if (us) {fb_write("user-mode ");}
   if (reserved) {fb_write("reserved ");}
   fb_write(" at ");
   fb_write_hex(faulting_address);
   fb_write("\n");
   PANIC("Page fault");
}
