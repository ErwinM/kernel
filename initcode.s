; Initial process execs /init.

; include "syscall.h"
; include "traps.h"


; exec(init, argv)
global startinitcode
startinitcode:
	push dword argv
  push dword init
  push dword 0  ; where caller pc would be
  mov eax, 7 ; 7 = SYS_EXEC
	int 64 ; 64 = system call

# for(;;) exit();
exit:
  mov eax, 2 ; 2 = SYS_EXIT
  int 64
  jmp exit

# char init[] = "/init\0";
init:
  db "/init\0"

# char *argv[] = { init, 0 };
ALIGN 2
argv:
  dd init
  dd 0
