/* See LICENSE file for copyright and license details. */

#include "sched.h"

#include "csr.h"
#include "kassert.h"
#include "kernel.h"
#include "kprint.h"
#include "proc.h"
#include "rtc.h"
#include "trap.h"
#include "wfi.h"

typedef struct slot_info {
	// Owner of time slot.
	uint32_t pid;
	// Remaining length of corresponding slice.
	uint32_t length;
} slot_info_t;

struct sched_decision {
	proc_t *proc;
	uint64_t end_time;
};

static uint64_t slots[S3K_SLOT_CNT];

void sched_init(void)
{
	uint64_t pid = 0;
	uint64_t end = S3K_SLOT_CNT;
	uint64_t from = 0;
	uint64_t to = S3K_SLOT_CNT;

	sched_update(pid, end, from, to);
}

void sched_update(uint64_t pid, uint64_t end, uint64_t from, uint64_t to)
{
	uint64_t mask = 0xFFFFull;
	for (uint64_t i = from; i < to; i++) {
		slots[i] &= ~mask;
		slots[i] |= ((pid << 8) | (end - i));
	}
}

void sched_delete(uint64_t from, uint64_t to)
{
	uint64_t mask = 0xFFFFull;
	for (uint64_t i = from; i < to; ++i)
		slots[i] &= ~mask;
}

static void slot_info_get(uint64_t slot, slot_info_t *si)
{
	uint64_t entry = slots[slot % S3K_SLOT_CNT];
	si->pid = (entry >> 8) & 0xFF;
	si->length = entry & 0xFF;
}

proc_t *sched_fetch(void)
{
	slot_info_t si;
	uint64_t slot = rtc_time_get() / S3K_SLOT_LEN;
	slot_info_get(slot, &si);

	if (si.length == 0) {
		return NULL;
	}

	proc_t *proc = proc_get(si.pid);

	if (!proc_acquire(proc)) {
		return NULL;
	}

	proc->timeout = (slot + si.length) * S3K_SLOT_LEN;
	return proc;
}

proc_t *sched(void)
{
	return sched_fetch();
}

proc_t *sched_initial(void)
{
	// Process to schedule
	proc_t *proc = sched_fetch();
	proc_pmp_sync(proc);
	rtc_timeout_set(0, proc->timeout);
	return proc;
}
