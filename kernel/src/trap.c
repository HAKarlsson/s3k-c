#include <libkernel.h>
#include <sched.h>
#include <trap.h>
#include "rtc.h"
#include "kprint.h"

extern kstate_t ks;

proc_t *trap_handler(proc_t *proc, u64 mcause, u64 mtval)
{
	Trap_handler(&ks, mcause, mtval);

	// If the same process is scheduled, return it.
	if (ks.next_pid == proc->pid) {
		return proc;
	}

	// If not the same process, release the old process.
	Proc_release(proc);

	// If the next process is Proc_NULL, fetch the next process to run.
	while (ks.next_pid == Proc_NULL)
		Sched_fetch(&ks);
	
	// Prepare the next process to run.
	proc_t *next_proc = proc_get(ks.next_pid);
	proc_pmp_sync(next_proc);
	rtc_timeout_set(0, next_proc->timeout);
	return next_proc;
}
