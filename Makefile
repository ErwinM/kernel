OBJECTS = loader.o kmain.o kernel.o write.o descriptor_tables.o trap.o common.o timer.o proc.o alloc.o vm.o spinlock.o switch.o initcode.o initrd.o fs.o file.o
    CC = gcc
    CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
             -nostartfiles -nodefaultlibs -Wall -Wextra -c
    LDFLAGS = -T link.ld -melf_i386
    AS = nasm
    ASFLAGS = -f elf

all: kernel.elf

initcode:
	$(AS) $(ASFLAGS) initcode.s -o initcode.o
	#ld $(LDFLAGS) -N -e start -Ttext 0 -o initcode initcode.o
	#objcopy --input binary --output elf32-i386 --binary-architecture i386 initcode.out initcode
	# $(OBJDUMP) -S initcode.o > initcode.asm

	#--input binary \
	#				 --output elf32-i386 \
	#				 --binary-architecture i386 data.txt data.o


kernel.elf: $(OBJECTS)
	ld -T link.ld -melf_i386 $(OBJECTS) -o kernel.elf #initcode.out

mk_ramdsk: mk_ramdsk.c
	gcc -o mk_ramdsk mk_ramdsk.c


# 	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

#	kernel: $(OBJS) entry.o entryother initcode kernel.ld
# $(LD) $(LDFLAGS) -T kernel.ld -o kernel entry.o $(OBJS) -b binary initcode entryother
# $(OBJDUMP) -S kernel > kernel.asm
# $(OBJDUMP) -t kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel.sym


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
