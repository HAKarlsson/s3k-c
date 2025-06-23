/* See LICENSE file for copyright and license details. */
#include "proc.h"

#include "csr.h"
#include "kassert.h"
#include "libkernel.h"

extern struct Types_kstate ks;

void proc_init(word_t payload)
{
	for (u64 i = 0; i < S3K_PROC_CNT; i++) {
		ks.ptable[i]->pid = i;
		ks.ptable[i]->state = Proc_PSF_SUSPENDED;
	}
	ks.ptable[0]->state = 0;
	ks.ptable[0]->pc = (word_t)payload;
}

proc_t *proc_get(pid_t pid)
{
	KASSERT(pid < S3K_PROC_CNT);
	KASSERT(ks.ptable[pid]->pid == pid);
	return ks.ptable[pid];
}

bool proc_acquire(proc_t *proc)
{
	Ptable_acquire(&ks, proc->pid);
	return ks.vregs[0];
}

void proc_release(proc_t *proc)
{
	Ptable_release(&ks, proc->pid);
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
	u64 pmpcfg = 0;
	for (u64 i = 0; i < S3K_PMP_CNT; i++) {
		pmpcfg |= (u64)proc->pmpcfg[i] << (i * 8);
	}
	csrw_pmpcfg0(pmpcfg);
	return proc;
}
