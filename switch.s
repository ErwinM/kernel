; Context switch
;
;   void swtch(struct context **old, struct context *new);
;
; Save current register context in old
; and then load register context from new.

global swtch
swtch:
  mov eax, [esp+4]
  mov edx, [esp+8]

  ; Save old callee-save registers
  push ebp
  push ebx
  push esi
  push edi

  ; Switch stacks
  mov [eax], esp
  mov esp, edx

  ; Load new callee-save registers
  pop edi
  pop esi
  pop ebx
  pop ebp
  ret
