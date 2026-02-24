	.file	"multiplyBy1.cpp"
	.text
	.globl	_Z11multiplyBy1P8IntArray
	.type	_Z11multiplyBy1P8IntArray, @function
_Z11multiplyBy1P8IntArray:
.LFB0:
	.cfi_startproc
	endbr64
	cmpl	$0, (%rdi)
	jle	.L1
	movl	$0, %eax
.L3:
	movq	8(%rdi), %rdx
	leaq	(%rdx,%rax,4), %rdx
	movl	(%rdx), %ecx
	movl	%ecx, (%rdx)
	addq	$1, %rax
	cmpl	%eax, (%rdi)
	jg	.L3
.L1:
	ret
	.cfi_endproc
.LFE0:
	.size	_Z11multiplyBy1P8IntArray, .-_Z11multiplyBy1P8IntArray
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
