global user_mode_enter
global user_registers
global restore_general_purpose_registers

user_mode_enter_old:
    mov ebp, esp
    mov ebx, [ebp+4] ; FUNCTION POINTER FOR USER LAND TO RUN
    mov eax, [ebp+8] ; Stack address for this process
  
    cli
    push eax        ; Save stack address for later
    call user_registers ; Switch to the user registers
    pop eax        ; Restore the stack address, we need to push it to the stack


    push dword [ebp+12] 
    push dword eax

    pushf
    pop eax
    or eax, 0x200
    push eax
    push 0x1b
    push ebx ; FUNCTION TO RUN IN USER LAND
    iret



; uint32_t edi;
  ;  uint32_t esi;
  ;  uint32_t ebp;
  ;  uint32_t ebx;
  ;  uint32_t edx;
  ;;  uint32_t ecx;
  ;  uint32_t eax;

   ; uint32_t ip;
  ;  uint32_t cs;
  ;  uint32_t flags;
   ; uint32_t esp;
   ; uint32_t ss;


user_mode_enter:
    push esp
    mov ebp, esp
    push dword [ebp+8] ; The pointer to the registers
    call restore_general_purpose_registers
    pop esp
    ret

; void restore_general_purpose_registers(struct registers* registers)
restore_general_purpose_registers:
    push esp
    mov ebp, esp
    mov ebx, [ebp+8]
    mov edi, [ebx]
    mov esi, [ebx+4]
    mov ebp, [ebx+8]
    mov ebx, [ebx+12]
    mov edx, [ebx+16]
    mov ecx, [ebx+20]
    mov eax, [ebx+24]
    pop esp
    ret

user_registers:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret