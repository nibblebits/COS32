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


gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start -1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

DAPACK:
	db	0x10
	db	0
blkcnt:	dw	50		; int 13 resets this to # of blocks actually read/written
db_add:	dw	0x7e00		 ;memory buffer destination address (0:3ff)
	dw	0x00		; in memory page zero
d_lba:	dd	1		; put the lba to read in this spot
	dd	0		; more storage bytes only for big lba's ( > 4 bytes )


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

    mov bp, 0x9000          ; Set  the  stack.
    mov sp, bp
    mov si, DAPACK
;    mov bx, 0x7e00
    ;mov word [db_add], bx

    mov ah, 0x42
    int 0x13
    jc .problem
    jmp .load_protected
.problem:
    mov si, problem_loading_kernel
    call print
    jmp $

.load_protected:    
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:0x7e00


problem_loading_kernel: db 'Problem loading kernel', 0
TIMES 510-($-$$) db 0
dw 0xAA55
