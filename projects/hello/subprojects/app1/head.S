.globl _start
.type _start, @function
_start:
	// Set up the global pointer.
	.option push
	.option norelax
	la	t0,__global_pointer$
	.option pop
	// Set up the stack pointer.
	la	sp,__stack_top

	li	a0,0
	li	a1,0
	call	main
__hang:
	j	__hang
	