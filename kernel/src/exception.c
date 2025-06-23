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
	proc->ecause = mcause;
	proc->eval = mtval;
	proc->epc = proc->pc;
	proc->esp = proc->sp;
	proc->pc = proc->tpc;
	proc->sp = proc->tsp;
	return proc;
}

static proc_t *_exception_trap_return(proc_t *proc)
{
	proc->pc = proc->epc;
	proc->sp = proc->esp;
	proc->ecause = 0;
	proc->eval = 0;
	proc->epc = 0;
	proc->esp = 0;
	return proc;
}

proc_t *exception_handler(proc_t *proc, uint64_t mcause, uint64_t mtval)
{
	kprintf("Exception: mcause: %X, mtval: %X, mepc: %X\n", mcause, mtval,
		proc->pc);
	// while (1) {
	// }
	if (mcause == ILLEGAL_INSTRUCTION
	    && (mtval == MRET || mtval == SRET || mtval == URET))
		return _exception_trap_return(proc);
	return _exception_delegate(proc, mcause, mtval);
}
