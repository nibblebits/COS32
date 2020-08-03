[BITS 32]
section .asm

global task_return
global user_registers
global restore_general_purpose_registers


; uint32_t edi; 0
  ;  uint32_t esi; 4
  ;  uint32_t ebp; 8
  ;  uint32_t ebx; 12
  ;  uint32_t edx; 16
  ;;  uint32_t ecx; 20
 ;  uint32_t eax; 24

   ; uint32_t ip; 28
  ;  uint32_t cs; 32 
  ;  uint32_t flags; 36
   ; uint32_t esp; 40
   ; uint32_t ss; 44


task_return:
    mov ebp, esp
    ; PUSH THE DATA SEGMENT (SS WILL DO FINE WE USE THE SAME)
    ; PUSH THE STACK ADDRESS
    ; PUSH THE FLAGS
    ; PUSH THE CODE SEGMENT
    ; PUSH IP

    ; Let's access the structure passed to us
    mov ebx, [ebp+4]
    
    ; Push the data/stack selector one and the same in our implemention
    push dword[ebx+44]
    ; Push the stack pointer
    push dword[ebx+40]

    ; Push the flags
    ; NOTE WE NEED TO RESTORE FLAGS AT SOME POINT ALSO!!!! FOR NOW WE WILL PUSH WHAT WE HAVE! NEEDS IMPROVEMENT!!!!
    ; HERE WE SET THE INTERRUPT FLAG AS WE CAN'T DO THIS IN USER LAND
    pushf
    pop eax
    or eax, 0x200
    push eax

    ; Push the code segment
    push dword[ebx+32]

    ; Push the IP we want to execute
    push dword[ebx+28]

    ; We want to setup the segment registers. WIll work I Hope?
    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; NO need to change the stack segment!!! This happens when we return from interrupt
    ; We don't want to fault!


    ; Last thing to do is restore the registers provided to us
    push dword[ebp+4]
    call restore_general_purpose_registers
    ; We can't pop this or we will corrupt a register, lets adjust the stack pointer to discard
    add esp, 4

    ; Let's leave and execute!
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