; descriptor_tables.s
; Loads GDT table using lgdt and performas jump to code segment

global gdt_flush

; stack:
;		[esp + 4] = pointer to gdt_ptr_struct

gdt_flush:
	mov eax, [esp + 4]
	lgdt[eax]

	mov ax, 0x10 ; point all data registers at data segment entry in GDT at 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	jmp 0x08:.flush ; jump to code segment entry at 0x08
.flush:
	ret
