
; This file is responsible for calling the C main function of the program whos linking against us
; 23 Aug 2020 23:19

global _start
extern start_c_func
extern start_get_arguments

_start:    
    call start_c_func
    ret

; int start_get_arguments(struct process_arguments* arguments);
start_get_arguments:
    push ebp
    mov ebp, esp
    mov eax, 19 ; Command 19 get the process arguments, argc, and argv.
    push dword [ebp+8] ; The process_arguments structure to be populated
    int 0x80    ; Invoke the kernel!
    add esp, 4
    pop ebp
    ret