#include "kprint.h"

#include <interrupt.h>
#include <proc.h>
#include <sched.h>

// proc_t *interrupt_handler(proc_t *proc, uint64_t mcause, uint64_t mtval)
// {
// 	(void)mcause;
// 	(void)mtval;
// 	proc_release(proc);

// 	return sched();
// }

proc_t *interrupt_handler(proc_t *proc, uint64_t mcause, uint64_t mtval)
{
	(void)mcause;
	(void)mtval;
	return NULL;
}
