; Port communication via mapped memory
global outb

; outb - send a byte to an I/O port
; stack:	[esp+8] the data bytes
;					[esp+4] the I/O port
;					[esp	] return address
outb:
	mov al, [esp+8]	; move the data to be sent into the al register
	mov dx, [esp+4]	; move the address of the I/O port to the dx register
	out dx, al			; send the data to the I/O port
	ret							; return to the calling function


; Loads GDT table using lgdt and performas jump to code segment
global gdt_flush

; gdt_flush - load the GTD table
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

global idt_flush	    ; Allows the C code to call idt_flush().

idt_flush:
   mov eax, [esp+4]  ; Get the pointer to the IDT, passed as a parameter.
   lidt [eax]        ; Load the IDT pointer.
   ret


; Interrupt Service Request Handles
; We define 2 macro's: one for exceptions that do not push an error code
; and one for exceptions that do push an error code
; NASM will convert these to the 32 appropriate code entries

%macro ISR_NOERRORCODE 1
global isr%1
isr%1:
	cli
	push byte 0		; push a dummy error code
	push byte %1		; push the interrup number
	jmp alltraps
%endmacro

%macro ISR_WITHERRORCODE 1
global isr%1
isr%1:
	cli
	push byte %1		; only push interrupt number, error code has already been pushed
	jmp alltraps
%endmacro

; This macro creates a stub for an IRQ - the first parameter is
; the IRQ number, the second is the ISR number it is remapped to.
%macro IRQ 2
global irq%1
irq%1:
	cli
	push byte 0
	push byte %2
	jmp alltraps
%endmacro

; now use the macro's to define the right handlers
ISR_NOERRORCODE 0
ISR_NOERRORCODE 1
ISR_NOERRORCODE 2
ISR_NOERRORCODE 3
ISR_NOERRORCODE 4
ISR_NOERRORCODE 5
ISR_NOERRORCODE 6
ISR_NOERRORCODE 7
ISR_WITHERRORCODE 8
ISR_NOERRORCODE 9
ISR_WITHERRORCODE 10
ISR_WITHERRORCODE 11
ISR_WITHERRORCODE 12
ISR_WITHERRORCODE 13
ISR_WITHERRORCODE 14
ISR_NOERRORCODE 15
ISR_NOERRORCODE 16
ISR_NOERRORCODE 17
ISR_NOERRORCODE 18
ISR_NOERRORCODE 19
ISR_NOERRORCODE 20
ISR_NOERRORCODE 21
ISR_NOERRORCODE 22
ISR_NOERRORCODE 23
ISR_NOERRORCODE 24
ISR_NOERRORCODE 25
ISR_NOERRORCODE 26
ISR_NOERRORCODE 27
ISR_NOERRORCODE 28
ISR_NOERRORCODE 29
ISR_NOERRORCODE 30
ISR_NOERRORCODE 31
IRQ   0,    32
IRQ   1,    33
IRQ   2,    34
IRQ   3,    35
IRQ   4,    36
IRQ   5,    37
IRQ   6,    38
IRQ   7,    39
IRQ   8,    40
IRQ   9,    41
IRQ  10,    42
IRQ  11,    43
IRQ  12,    44
IRQ  13,    45
IRQ  14,    46
IRQ  15,    47

; In isr.c
extern trap
;extern irq_handler

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
alltraps:
  ; Build trap frame.
  push ds
  push es
  push fs
  push gs
  pusha

  ; Set up data and per-cpu segments.
  mov ax, 0x10
  mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

  ; Call trap(tf), where tf=%esp
  push esp
  call trap
  add esp, 4

  ; Return falls through to trapret...
global trapret
trapret:
  popa
  pop gs
  pop fs
  pop es
  pop ds
  add esp, 0x8  ; trapno and errcode
	sti
  iret

 ; This is our common IRQ stub. It saves the processor state, sets
 ; up for kernel mode segments, calls the C-level fault handler,
 ; and finally restores the stack frame.
 irq_common_stub:
	 ; Build trap frame.
	 push ds
	 push es
	 push fs
	 push gs
	 pusha

	 ; Set up data and per-cpu segments.
	 mov ax, 0x10
	 mov ds, ax
	 mov es, ax
	 mov fs, ax
	 mov gs, ax

	 ; Call trap(tf), where tf=%esp
	 push esp
	 call trap
	 add esp, 4

	 ; Return falls through to trapret...
	 popa
	 pop gs
	 pop fs
	 pop es
	 pop ds
	 add esp, 0x8  ; trapno and errcode
	 sti
	 iret
