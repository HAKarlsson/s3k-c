/* See LICENSE file for copyright and license details. */
#include "exception.h"

#include "kprint.h"
#include "proc.h"

#define ILLEGAL_INSTRUCTION 0x2

#define MRET 0x30200073
#define SRET 0x10200073
#define URET 0x00200073

static proc_t *_exception_delegate(proc_t *proc, uint64_t mcause,
				   uint64_t mtval)
{
	proc->regs.ecause = mcause;
	proc->regs.eval = mtval;
	proc->regs.epc = proc->regs.pc;
	proc->regs.esp = proc->regs.sp;
	proc->regs.pc = proc->regs.tpc;
	proc->regs.sp = proc->regs.tsp;
	return proc;
}

static proc_t *_exception_trap_return(proc_t *proc)
{
	proc->regs.pc = proc->regs.epc;
	proc->regs.sp = proc->regs.esp;
	proc->regs.ecause = 0;
	proc->regs.eval = 0;
	proc->regs.epc = 0;
	proc->regs.esp = 0;
	return proc;
}

proc_t *exception_handler(proc_t *proc, uint64_t mcause, uint64_t mtval)
{
#ifndef NDEBUG
	kprintf("Exception: mcause: %X, mtval: %X, mepc: %X\n", mcause, mtval,
		proc->regs.pc);
#endif
	while (1) {
	}
	if (mcause == ILLEGAL_INSTRUCTION
	    && (mtval == MRET || mtval == SRET || mtval == URET))
		return _exception_trap_return(proc);
	return _exception_delegate(proc, mcause, mtval);
}
