
; This file is responsible for calling the C main function of the program whos linking against us
; 23 Aug 2020 23:19

global _start
extern start_c_func

_start:    
    call start_c_func
    ret
