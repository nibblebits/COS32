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



; uint32_t edi; 0
  ;  uint32_t esi; 4
  ;  uint32_t ebp; 8
  ;  uint32_t ebx; 12
  ;  uint32_t edx; 16
  ;;  uint32_t ecx; 20
 ;  uint32_t eax; 24

   ; uint32_t ip; 28
  ;  uint32_t cs; 32 
  ;  uint32_t flags;
   ; uint32_t esp;
   ; uint32_t ss;




user_mode_enter:
    mov ebp, esp

    ; NOTE WE NEED TO RESTORE FLAGS AT SOME POINT ALSO!!!! FOR NOW WE WILL PUSH WHAT WE HAVE! NEEDS IMPROVEMENT!!!!
    ; HERE WE SET THE INTERRUPT FLAG AS WE CAN'T DO THIS IN USER LAND
    pushf
    pop eax
    or eax, 0x200
    push eax

    ; Let's access the structure passed to us
    mov ebx, [ebp+4]
    ; Now comes the code segment. 
    push dword [ebx+32]
    
    ; Now comes the IP
    push dword [ebx+28]

    ; Restore the general purpose registers
    push dword [ebp+4] ; The pointer to the registers
    call restore_general_purpose_registers
    ; Let's discard the last item
    add esp, 4

    ; Consider using popa in future to avoid these complications

    iretd

; void restore_general_purpose_registers(struct registers* registers)
restore_general_purpose_registers:
    push esp
    mov ebp, esp
    mov ebx, [ebp+8]
    mov edi, [ebx]
    mov esi, [ebx+4]
    mov ebp, [ebx+8]
    mov edx, [ebx+16]
    mov ecx, [ebx+20]
    mov eax, [ebx+24]
    mov ebx, [ebx+12]
    pop esp
    ret

user_registers:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret