	.file	"multiplyByX.cpp"
	.text
	.globl	_Z11multiplyByXP8IntArrayi
	.type	_Z11multiplyByXP8IntArrayi, @function
_Z11multiplyByXP8IntArrayi:
.LFB0:
	.cfi_startproc
	endbr64
	cmpl	$0, (%rdi)
	jle	.L1
	movl	$0, %eax
.L3:
	movq	8(%rdi), %rdx
	leaq	(%rdx,%rax,4), %rdx
	movl	%esi, %ecx
	imull	(%rdx), %ecx
	movl	%ecx, (%rdx)
	addq	$1, %rax
	cmpl	%eax, (%rdi)
	jg	.L3
.L1:
	ret
	.cfi_endproc
.LFE0:
	.size	_Z11multiplyByXP8IntArrayi, .-_Z11multiplyByXP8IntArrayi
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
