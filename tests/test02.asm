; Test02: add some data variables and accesses

	BITS	32

SECTION .data

_var1	db	1
_var2	dw	2
_var3	dd	4

SECTION .bss

_bss1	resb	3
_bss2	resw	2
_bss3	resd	1

SECTION .text

GLOBAL _main
_main
	movzx	ebx, byte [_var1]
	mov	ecx, _var2
	mov	[_var3], ecx

	movsx	edx, byte [_bss1]
	mov	eax, _bss2
	mov	[_bss3], eax

	xor	eax, eax
	ret

