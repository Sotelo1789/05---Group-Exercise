	.file	"multiplyBy61.cpp"
	.text
	.globl	_Z12multiplyBy61P8IntArray
	.type	_Z12multiplyBy61P8IntArray, @function
_Z12multiplyBy61P8IntArray:
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
	movl	%ecx, %r8d
	movl	%ecx, %r9d
	sall	$2, %r9d
	addl	%r9d, %r8d
	movl	%ecx, %r9d
	sall	$3, %r9d
	addl	%r9d, %r8d
	movl	%ecx, %r9d
	sall	$4, %r9d
	addl	%r9d, %r8d
	movl	%ecx, %r9d
	sall	$5, %r9d
	addl	%r9d, %r8d
	movl	%r8d, (%rdx)
	addq	$1, %rax
	cmpl	%eax, (%rdi)
	jg	.L3
.L1:
	ret
	.cfi_endproc
.LFE0:
	.size	_Z12multiplyBy61P8IntArray, .-_Z12multiplyBy61P8IntArray
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
