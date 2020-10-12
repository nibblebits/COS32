
; This file is responsible for calling the C main function of the program whos linking against us
; 23 Aug 2020 23:19

global _start
extern main

_start:
    ; ARGV AND ARGC were pushed automatically when starting this program
    call main
    ret
