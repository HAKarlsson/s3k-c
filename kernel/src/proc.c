/* See LICENSE file for copyright and license details. */
#include "proc.h"

#include "csr.h"
#include "kassert.h"
#include "libkernel.h"

extern kstate_t ks;

void proc_init(word_t payload)
{
	for (u64 i = 0; i < S3K_PROC_CNT; i++) {
		ks.ptable[i].pid = i;
		ks.ptable[i].state = Proc_psf_suspended;
	}
	ks.ptable[0].state = 0;
	// ks.ptable[0].pc = (word_t)payload;
	ks.ptable[0].regs[(u64)Types_Reg_pc] = (word_t)payload;
}

proc_t *proc_get_opt(pid_t pid)
{
	return pid == Proc_pid_null ? NULL : &ks.ptable[pid];
}

proc_t *proc_get(pid_t pid)
{
	KASSERT(pid < S3K_PROC_CNT);
	KASSERT(ks.ptable[pid].pid == pid);
	return &ks.ptable[pid];
}

proc_t *proc_pmp_sync(proc_t *proc)
{
	csrw_pmpaddr0(proc->pmpaddr[0]);
	csrw_pmpaddr1(proc->pmpaddr[1]);
	csrw_pmpaddr2(proc->pmpaddr[2]);
	csrw_pmpaddr3(proc->pmpaddr[3]);
	csrw_pmpaddr4(proc->pmpaddr[4]);
	csrw_pmpaddr5(proc->pmpaddr[5]);
	csrw_pmpaddr6(proc->pmpaddr[6]);
	csrw_pmpaddr7(proc->pmpaddr[7]);
	csrw_pmpcfg0(proc->pmpcfg);
	return proc;
}
