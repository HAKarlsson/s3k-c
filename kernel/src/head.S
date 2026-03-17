// See LICENSE file for copyright and license details.
#include "offsets.h"
#include "csr.h"

.extern kernel_init
.extern trap_entry
.extern trap_return

.section .text.init,"ax",@progbits

.globl _start
.type _start, @function
_start:
head_entry:
	// Set up the global pointer.
	.option push
	.option norelax
	la	t0,__global_pointer$
	.option pop
	// Set up the stack pointer.
	la	sp,__stack_top

	// If hartid != 0, then jump to __hang.
	csrr	t0,mhartid
	bnez	t0,__hang

	// Set some CSRs to 0.
	csrw	mstatus,0
	csrw	medeleg,0
	csrw	mideleg,0
	csrw	mscratch,0
	csrw	mie,0
	csrw	satp,0
	csrw	mcounteren,0xf
	csrw	mcountinhibit,0x0
	csrw	scounteren,0xf

	// Set trap entry.
	la	t0,__hang
	csrw	mtvec,t0

zero_bss:
	// Initialize the bss section.
	la	t0,_bss
	la 	t1,_end
	j	2f
1:	sb	zero,(t0)
	addi	t0,t0,1
2:	bne	t0,t1,1b

	call	kernel_init

head_exit:
	// Enable timer interrupts.
	li	t0,MIE_MTIE
	csrw	mie,t0

	// Start user processes.
	la	t0,trap_entry
	csrw	mtvec,t0
	csrw	mscratch,0
	call 	sched_initial
	tail	trap_return

__hang:
	wfi
	j	__hang
