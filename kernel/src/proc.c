/* See LICENSE file for copyright and license details. */
#include "proc.h"

#include "cap/pmp.h"
#include "csr.h"
#include "kassert.h"
#include "libkernel.h"
#include "rtc.h"

extern struct Kernel_state ks;

static uint64_t _pmpcfg[S3K_PROC_CNT][S3K_PMP_CNT];
static uint64_t _pmpaddr[S3K_PROC_CNT][S3K_PMP_CNT];
static proc_t _procs[S3K_PROC_CNT];
static proc_t *_ptable[S3K_PROC_CNT];

void proc_init(word_t payload)
{
	for (uint64_t i = 0; i < S3K_PROC_CNT; i++) {
		_procs[i].pid = i;
		_procs[i].state = PSF_SUSPENDED;
		_procs[i].pmpcfg = (u64 *)_pmpcfg[i];
		_procs[i].pmpaddr = (u64 *)_pmpaddr[i];
		_ptable[i] = &_procs[i];
	}
	_procs[0].state = 0;
	_procs[0].pc = (word_t)payload;

	ks.ptable = _ptable;
}

proc_t *proc_get(pid_t pid)
{
	KASSERT(pid < S3K_PROC_CNT);
	KASSERT(ks.ptable[pid]->pid == pid);
	return ks.ptable[pid];
}

proc_state_t proc_get_state(proc_t *proc)
{
	proc_state_t state = proc->state;
	if ((state == PSF_BLOCKED) && rtc_time_get() >= proc->timeout)
		return 0;
	return state;
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

void proc_suspend(proc_t *proc)
{
	Ptable_suspend(&ks, proc->pid);
}

void proc_resume(proc_t *proc)
{
	Ptable_resume(&ks, proc->pid);
}

void proc_ipc_wait(proc_t *proc, chan_t chan)
{
	KASSERT(proc->state == PSF_BUSY);
	proc->state = PSF_BLOCKED | ((uint64_t)chan << 48) | PSF_BUSY;
}

bool proc_ipc_acquire(proc_t *proc, chan_t chan)
{
	proc_state_t expected = PSF_BLOCKED | ((uint64_t)chan << 48);
	proc_state_t desired = PSF_BUSY;

	if (proc->state != expected)
		return false;
	if (rtc_time_get() >= proc->timeout)
		return false;
	proc->state = desired;
	return true;
}

bool proc_is_suspended(proc_t *proc)
{
	return proc->state == PSF_SUSPENDED;
}

bool proc_pmp_avail(proc_t *proc, pmp_slot_t slot)
{
	return Ptable_pmp_avail(&ks, proc->pid, slot);
}

void proc_pmp_load(proc_t *proc, pmp_slot_t slot, pmp_slot_t rwx, napot_t addr)
{
	Ptable_pmp_load(&ks, proc->pid, slot, rwx, addr);
}

void proc_pmp_unload(proc_t *proc, pmp_slot_t slot)
{
	Ptable_pmp_unload(&ks, proc->pid, slot);
}

proc_t *proc_pmp_sync(proc_t *proc)
{
	// kprintf("Resuming proc %d\n", proc->pid);
	csrw_pmpaddr0(proc->pmpaddr[0]);
	csrw_pmpaddr1(proc->pmpaddr[1]);
	csrw_pmpaddr2(proc->pmpaddr[2]);
	csrw_pmpaddr3(proc->pmpaddr[3]);
	csrw_pmpaddr4(proc->pmpaddr[4]);
	csrw_pmpaddr5(proc->pmpaddr[5]);
	csrw_pmpaddr6(proc->pmpaddr[6]);
	csrw_pmpaddr7(proc->pmpaddr[7]);
	uint64_t pmpcfg = 0;
	for (int i = 0; i < S3K_PMP_CNT; i++) {
		pmpcfg |= (uint64_t)proc->pmpcfg[i] << (i * 8);
	}
	csrw_pmpcfg0(pmpcfg);
	return proc;
}
