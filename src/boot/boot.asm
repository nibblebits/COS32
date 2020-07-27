[bits 16]
[org 0x7c00]

jmp short _start
nop

; FAT16 header  
OEMIdentifier   db 'COS32   '
BytesPerSector 		dw 0x200	; 512 bytes per sector
SectorsPerCluster	db 0x80		;
ReservedSectors 	dw 50		; Reserved sectors before FAT (TODO: is this BOOT?)
FATCopies		db 0x02		  ; Often this value is 2.
RootDirEntries 		dw 0x40 	; 64 Root directory entries
NumSectors		dw 0x0000	;  If this value is 0, it means there are more than 65535 sectors in the volume
MediaType		db 0xF8		; Fixed disk -> Harddisk
SectorsPerFAT		dw 0x0100	; Sectors used by each FAT Table
SectorsPerTrack		dw 0x20		; TODO: Look up? BIOS might change those
NumberOfHeads		dw 0x40		; Does this even matter?
HiddenSectors		dd 0x00
SectorsBig		dd 0x773594		;

; Extended BPB (DOS 4.0)
DriveNumber		db 0x80		; 0 for removable, 0x80 for hard-drive
WinNTBit		db 0x00		; WinNT uses this
Signature		db 0x29		; DOS 4.0 EBPB signature
VolumeID		dd 0x0000D105	; Volume ID
VolumeIDString		db "COS32 BOOT "; Volume ID
SystemIDString		db "FAT16   "   ; File system type, pad with blanks to 8 bytes


struc gdt_entry_struct

	limit_low:   resb 2
	base_low:    resb 2
	base_middle: resb 1
	access:      resb 1
	granularity: resb 1
	base_high:   resb 1

endstruc

_start:
jmp 0:kernel_start


tss_struct:
    dd 0x00 ; LINK
    dd 0x00400000 ; ESP0 - Kernel stack pointer
    dd 0x08 ; SS0 Kernel stack segment used on ring transitions
    dd 0x00 ; ESP1
    dd 0x00 ; ESP2
    dd 0x00 ; SS2
    dd 0x00 ; SR3
    dd 0x00 ; EIP
    dd 0x00 ; EFLAGS
    dd 0x00 ; EAX
    dd 0x00 ; ECX
    dd 0x00 ; EDX
    dd 0x00 ; EBX
    dd 0x00 ; ESP
    dd 0x00 ; EBP
    dd 0x00 ; ESI
    dd 0x00 ; EDI
    dd 0x00 ; ES
    dd 0x00 ; CS
    dd 0x00 ; SS
    dd 0x00 ; DS
    dd 0x00 ; FS
    dd 0x00 ; GS
    dd 0x00 ; LDTR
    dd 0x00 ; IOPB offset
gdt_start:

gdt_null:
    dd 0x0
    dd 0x0

; offset 0x8
gdt_code:				; cs should point to this descriptor
	dw 0xffff		; segment limit first 0-15 bits
	dw 0			; base first 0-15 bits
	db 0			; base 16-23 bits
	db 0x9a			; access byte
	db 11001111b	; high 4 bits (flags) low 4 bits (limit 4 last bits)(limit is 20 bit wide)
	db 0			; base 24-31 bits

; offset 0x10
gdt_data:				; ds, ss, es, fs, and gs should point to this descriptor
	dw 0xffff		; segment limit first 0-15 bits
	dw 0			; base first 0-15 bits
	db 0			; base 16-23 bits
	db 0x92			; access byte
	db 11001111b	; high 4 bits (flags) low 4 bits (limit 4 last bits)(limit is 20 bit wide)
	db 0			; base 24-31 bits


 ;offset 0x18
;gdt_user_code:				; cs should point to this descriptor
;	dw 0xffff		; segment limit first 0-15 bits
;	dw 0			; base first 0-15 bits
;	db 0			; base 16-23 bits
;	db 0xfa			; access byte
;	db 11001111b	; high 4 bits (flags) low 4 bits (limit 4 last bits)(limit is 20 bit wide)
;	db 0			; base 24-31 bits
;
; offset 0x20
;gdt_user_data:				; ds, ss, es, fs, and gs should point to this descriptor
;	dw 0xffff		; segment limit first 0-15 bits;
 ;   dw 0			; base first 0-15 bits
;	db 0			; base 16-23 bits
;	db 0xf2			; access byte;
 ;   db 11001111b	; high 4 bits (flags) low 4 bits (limit 4 last bits)(limit is 20 bit wide)
;	db 0			; base 24-31 bits
;
; offset 0x28
;gdt_tss:			; ds, ss, es, fs, and gs should point to this descriptor
 ;   dw 0ffffh
  ;  dw tss_struct
   ; dd 0000e900h


gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start -1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

print:
    pusha
    mov ah, 14
    mov bh, 0
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

kernel_start:

.load_protected:    
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32

[BITS 32]


load32:
   ;  Load TSS
    ;mov ax, 0x28  ;The descriptor of the TSS in the GDT (e.g. 0x28 if the sixths entry in your GDT describes your TSS)
    ;ltr ax        ;The actual load

    ; Let's enable the A20 LINE

    in al, 0x92
    or al, 2
    out 0x92, al

    ; Load 120 sectors of the kernel
    mov eax, 1
    mov ecx, 120
    mov edi, 0x0100000
    call ata_lba_read

    ; Jump to the kernel!
    jmp CODE_SEG:0x0100000

    
; ECX - total sectors
; EDI - Destination
; EAX - LBA
ata_lba_read:
    mov ebx, eax ; Backup LBA
    ; SEND HIGHEST 8 BITS OF LBA TO HARD DISK CONTROLLER
    shr eax, 24
    or eax, 0xE0 ; OR It with 0xE0 to select master drive
    mov dx, 0x1F6
    out dx, al
    ; FINISHED SENDING 8 BITS OF LBA TO HARD DISK CONTROLLER
    
    ; SEND TOTAL SECTORS TO HARD DISK CONTROLLER
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; FINISHED SENDING TOTAL SECTORS

   ; SEND SOME MORE BITS OF LBA TO HARD DISK CONTROLLER
    mov dx, 0x1F3
    mov eax, ebx ; RESTORE ORIGINAL LBA
    out dx, al
    ; FINISHED SENDING USEND SOME MORE BITS OF LBA TO HARD DISK CONTROLLER

    ; SEND FINAL UPPER 8 BITS OF LBA TO HARD DISK CONTROLLER
    mov dx, 0x1F4
    mov eax, ebx ; RESTORE ORIGINAL LBA
    shr eax, 8
    out dx, al
    ; FINISHED SENDING UPPER 8 BITS OF LBA


    ; SEND FINAL UPPER 16 BITS OF LBA TO HARD DISK CONTROLLER
    mov dx, 0x1F5
    mov eax, ebx ; RESTORE ORIGINAL LBA
    shr eax, 16
    out dx, al
    ; FINISHED SENDING UPPER 16 BITS OF LBA

    mov dx, 0x1f7
    mov al, 0x20
    out dx, al



    ; READ ALL SECTORS INTO MEMORY
.next_sector:
    push ecx

; Check if we are ready to read
.try_again:
    mov dx, 0x1f7
    in al, dx
    test al, 8
    jz .try_again

    ; We will read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw
    pop ecx
    loop .next_sector
    ; END OF READING SECTORS INTO MEMORY

    ret

    

problem_loading_kernel: db 'Problem loading kernel', 0
TIMES 510-($-$$) db 0
dw 0xAA55
