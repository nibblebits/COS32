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


; CHANGE THIS TO A POINTER EVENTUALLY!!!!
user_mode_enter:
    push esp
    mov ebp, esp
    ; We could just pass the stack as it is but this would be a bad idea, could easily forget. We will 
    ; pass it down again. But switch to pointers eventually!
    push dword [ebp+32]
    push dword [ebp+28]
    push dword [ebp+24]
    push dword [ebp+20]
    push dword [ebp+16]
    push dword [ebp+12]
    push dword  [ebp+8]
    call restore_general_purpose_registers
    pop esp
    ret

restore_general_purpose_registers:
    push esp
    mov ebp, esp
    mov edi, [ebp+8]
    mov esi, [ebp+12]
    mov ebp, [ebp+16]
    mov ebx, [ebp+20]
    mov edx, [ebp+24]
    mov ecx, [ebp+28]
    mov eax, [ebp+32]
    pop esp
    ret

user_registers:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret