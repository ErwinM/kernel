OBJECTS = \
	loader.o\
	kmain.o\
	kernel.o\
	write.o\
	descriptor_tables.o\
	trap.o\
	common.o\
	timer.o\
	proc.o\
	alloc.o\
	vm.o\
	spinlock.o\
	switch.o\
	initrd.o\
	fs.o\
	file.o\
	syscall.o\
	sys_file.o\
	exec.o\
	kbd.o\
	pic.o\
	sysproc.o\
	console.o\

CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -c
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

all: kernel.elf
	#-nostdinc -I. -c

ULIB = ulib.o usys.o printf.o

init: init.o $(ULIB)
	ld -m elf_i386 -Ttext 0 -o init.elf init.o $(ULIB)
	./mk_ramdsk init.elf init.elf sh sh

sh: sh.o $(ULIB)
	ld -T ulink.ld -m elf_i386 -Ttext 0 -o sh sh.o $(ULIB)
	./mk_ramdsk init.elf init.elf sh sh



initcode:
	nasm initcode.s -o initcode.out
	ld -e start -r -b binary -m elf_i386 -Ttext 0 -o initcode.o initcode.out
	#objcopy -S -O binary initcode.out initcode


kernel.elf: $(OBJECTS) initcode
	ld -T link.ld -m elf_i386 -o kernel.elf $(OBJECTS) initcode.o

mk_ramdsk: mk_ramdsk.c
	gcc -o mk_ramdsk mk_ramdsk.c


os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	cp initrd.img iso/boot/initrd.img
	genisoimage -R                              \
                -b boot/grub/stage2_eltorito    \
                -no-emul-boot                   \
                -boot-load-size 4               \
                -A os                           \
                -input-charset utf8             \
                -quiet                          \
                -boot-info-table                \
                -o os.iso                       \
                iso

run: os.iso
	bochs -f bochsrc.txt -q

%.o: %.c
	$(CC) $(CFLAGS)  $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf *.o kernel.elf os.iso initcode initcode.out
