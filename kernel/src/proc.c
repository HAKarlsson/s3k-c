/* See LICENSE file for copyright and license details. */
#include "proc.h"

#include "cap/pmp.h"
#include "csr.h"
#include "kassert.h"
#include "rtc.h"

static uint32_t _pmpcfg[S3K_PROC_CNT][S3K_PMP_CNT];
static uint64_t _pmpaddr[S3K_PROC_CNT][S3K_PMP_CNT];
static proc_t procs[S3K_PROC_CNT];

void proc_init(word_t payload)
{
	for (uint64_t i = 0; i < S3K_PROC_CNT; i++) {
		procs[i].pid = i;
		procs[i].state = PSF_SUSPENDED;
		procs[i].pmpcfg = _pmpcfg[i];
		procs[i].pmpaddr = (long long unsigned int*)_pmpaddr[i];
	}
	procs[0].state = 0;
	procs[0].pc = (word_t)payload;
}

proc_t *proc_get(pid_t pid)
{
	KASSERT(pid < S3K_PROC_CNT);
	KASSERT(procs[pid].pid == pid);
	return &procs[pid];
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
	proc_state_t expected = proc->state;
	proc_state_t desired = PSF_BUSY;

	if (expected & (PSF_BUSY | PSF_SUSPENDED))
		return false;

	if (rtc_time_get() < proc->timeout)
		return false;
	proc->state = desired;
	return true;
}

void proc_release(proc_t *proc)
{
	KASSERT(proc->state & PSF_BUSY);
	proc->state &= ~PSF_BUSY;
}

void proc_suspend(proc_t *proc)
{
	proc_state_t prev = proc->state;
	proc->state |= PSF_SUSPENDED;

	if (prev & PSF_BLOCKED) {
		proc->state = PSF_SUSPENDED;
		proc->t0 = ERR_SUSPENDED;
	}
}

void proc_resume(proc_t *proc)
{
	if (proc->state == PSF_SUSPENDED)
		proc->timeout = 0;
	proc->state &= ~PSF_SUSPENDED;
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
	return proc->pmpcfg[slot] == 0;
}

void proc_pmp_load(proc_t *proc, pmp_slot_t slot, pmp_slot_t rwx, napot_t addr)
{
	proc->pmpcfg[slot] = rwx | 0x18;
	proc->pmpaddr[slot] = addr;
}

void proc_pmp_unload(proc_t *proc, pmp_slot_t slot)
{
	proc->pmpcfg[slot] = 0;
}

proc_t* proc_pmp_sync(proc_t *proc)
{
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
