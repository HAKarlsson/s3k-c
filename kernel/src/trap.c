#include <csr.h>
#include <exception.h>
#include <interrupt.h>
#include <rtc.h>
#include <sched.h>
#include <syscall.h>
#include <trap.h>

static proc_t *_handler(proc_t *proc, uint64_t mcause, uint64_t mtval)
{
	if (mcause == 8LL) {
		return syscall_handler(proc);
	} else if ((int64_t)mcause < 0LL) {
		return interrupt_handler(proc, mcause, mtval);
	} else {
		return exception_handler(proc, mcause, mtval);
	}
}

proc_t *trap_handler(proc_t *proc, uint64_t mcause, uint64_t mtval)
{
	proc_t *next = _handler(proc, mcause, mtval);

	if (next == proc) {
		return proc;
	}

	proc_release(proc);

	while (next == NULL) {
		next = sched();
	}

	proc_pmp_sync(next);
	rtc_timeout_set(csrr_mhartid(), next->timeout);
	return next;
}
