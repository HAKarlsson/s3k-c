#include "libkernel.h"
#include "syscall.h"
#include "csr.h"

extern kstate_t ks;

void Syscall_profiling_handler(pid_t pid)
{
	u64 begin = csrr_mcycle();
	Syscall_handler(pid);
	u64 end = csrr_mcycle();
	ks.syshandler_profiling[pid] = end - begin;
}
