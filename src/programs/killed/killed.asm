section .text

mov eax, message
int 0x80

jmp $

section .data
message: db 'The process was killed due to a run time error or external process'