#include "defs.h"
#include "common.h"
#include "elf.h"

extern struct proc *cp;

int exec(char *path, char **argv)
{
	struct inode *ip;
	struct elfhdr elf;
	struct proghdr ph;
	uint32_t argc, sp, ustack[3+MAXARG+1];
	char *s, *last;
	pde_t *pgdir, *oldpgdir;
	int k, off, sz;

	ip = namei(path);
	ilock(ip); // read the inode from disk

	// check file header
	if(readi(ip, (char*)&elf, 0, sizeof(elf)) < sizeof(elf))
		PANIC("exec: cannot read elf header");
	if(elf.magic != ELF_MAGIC)
		PANIC("exec: path does not point to elf header");

	pgdir = 0;
	if((pgdir=setupkvm())==0)
		PANIC("exec: setupkvm failed!");

	kprintf("exec: pgdir is at: %h", pgdir);
	sz = 0;
	for(k = 0, off = elf.phoff; k < elf.phnum; k++, off += sizeof(ph)){
		if(readi(ip, (char*)&ph, off, sizeof(ph)) < sizeof(ph))
			PANIC("exec: programhdrs corrupt");
		kprintf("exec: ELF header found!\n", 0);
		if(ph.type != ELF_PROG_LOAD)
			continue;
		sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz);
		// int loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint phoffset, uint sz)
		loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz);
	}
	lcr3(pgdir);
	bbrk();

	// setup user stack .....
	sz = PGROUNDUP(sz);
	if((sz = allocuvm(pgdir, sz, 2*PGSIZE)) < 2*PGSIZE)
		PANIC("exec: setup stack");
	sp = sz;

	// Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      PANIC("exec: MAXARG exceeded");
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      PANIC("exec: copyout failed");
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    PANIC("exec: copyout failed to copy ustack");

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(cp->name, last, sizeof(cp->name));

  // Commit to the user image.
  oldpgdir = cp->pgdir;
  cp->pgdir = pgdir;
  cp->sz = sz;
  cp->tf->eip = elf.entry;  // main
  cp->tf->esp = sp;
  switchuvm(cp);
  //freevm(oldpgdir);
  return 0;
}
