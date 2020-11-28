	.file	"testing.c"
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.align 16
	.globl	abc
	.type	abc, @function
abc:
.LFB0:
	.file 1 "./src/testing.c"
	.loc 1 2 1
	.cfi_startproc
	subl	$16, %esp
	.cfi_def_cfa_offset 20
	.loc 1 3 11
	movl	24(%esp), %eax
	movl	(%eax), %eax
	movl	%eax, 12(%esp)
	.loc 1 7 12
	movl	$0, %eax
	.loc 1 8 1
	addl	$16, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE0:
	.size	abc, .-abc
	.align 16
	.globl	test
	.type	test, @function
test:
.LFB1:
	.loc 1 11 1
	.cfi_startproc
	subl	$16, %esp
	.cfi_def_cfa_offset 20
	.loc 1 12 12
	movl	$0, 12(%esp)
	.loc 1 13 12
	pushl	12(%esp)
	.cfi_def_cfa_offset 24
	pushl	$50
	.cfi_def_cfa_offset 28
	call	abc
	addl	$8, %esp
	.cfi_def_cfa_offset 20
	.loc 1 14 1
	addl	$16, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE1:
	.size	test, .-test
.Letext0:
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0xa6
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF4
	.byte	0xc
	.long	.LASF5
	.long	.LASF6
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.long	.LASF1
	.byte	0x1
	.byte	0xa
	.byte	0x5
	.long	0x4f
	.long	.LFB1
	.long	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.long	0x4f
	.uleb128 0x3
	.long	.LASF3
	.byte	0x1
	.byte	0xc
	.byte	0xc
	.long	0x56
	.uleb128 0x2
	.byte	0x91
	.sleb128 -8
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x5
	.byte	0x4
	.long	0x5c
	.uleb128 0x5
	.byte	0x4
	.long	0x62
	.uleb128 0x6
	.byte	0x1
	.byte	0x6
	.long	.LASF0
	.uleb128 0x7
	.string	"abc"
	.byte	0x1
	.byte	0x1
	.byte	0x5
	.long	0x4f
	.long	.LFB0
	.long	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x8
	.string	"a"
	.byte	0x1
	.byte	0x1
	.byte	0xd
	.long	0x4f
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x9
	.long	.LASF2
	.byte	0x1
	.byte	0x1
	.byte	0x17
	.long	0x56
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0xa
	.string	"c"
	.byte	0x1
	.byte	0x3
	.byte	0xb
	.long	0x5c
	.uleb128 0x2
	.byte	0x91
	.sleb128 -8
	.byte	0
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x1c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0
	.value	0
	.value	0
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF5:
	.string	"./src/testing.c"
.LASF2:
	.string	"argv"
.LASF4:
	.string	"GNU C99 10.1.0 -mtune=generic -march=pentiumpro -g -O0 -O0 -std=gnu99 -falign-jumps -falign-functions -falign-labels -falign-loops -fomit-frame-pointer -finline-functions -fno-builtin -ffreestanding"
.LASF6:
	.string	"/home/daniel/COS32"
.LASF1:
	.string	"test"
.LASF0:
	.string	"char"
.LASF3:
	.string	"testing"
	.ident	"GCC: (GNU) 10.1.0"
