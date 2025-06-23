/* See LICENSE file for copyright and license details. */

#include "sched.h"

#include "csr.h"
#include "kassert.h"
#include "kprint.h"
#include "proc.h"
#include "rtc.h"
#include "trap.h"
#include "wfi.h"

typedef struct slot_info {
	// Owner of time slot.
	uint8_t pid;
	// Remaining length of corresponding slice.
	uint8_t length;
} slot_info_t;

struct sched_decision {
	proc_t *proc;
	uint64_t end_time;
};

extern struct Types_kstate ks;

struct Types_kstate *Sched_update(struct Types_kstate *ks, u64 pid, u64 end,
				  u64 from, u64 to)
{
	for (u64 i = from; i < to; i++) {
		ks->tslots[i] = ((pid << 8) | (end - i));
	}
	return ks;
}

struct Types_kstate *Sched_delete(struct Types_kstate *ks, u64 from, u64 to)
{
	u64 mask = 0xFFFFull;
	for (u64 i = from; i < to; ++i)
		ks->tslots[i] &= ~mask;
	return ks;
}

void sched_update(uint64_t pid, uint64_t end, uint64_t from, uint64_t to)
{
	Sched_update(&ks, pid, end, from, to);
}

void sched_delete(uint64_t from, uint64_t to)
{
	Sched_delete(&ks, from, to);
}

static void slot_info_get(uint64_t slot, slot_info_t *si)
{
	uint64_t entry = ks.tslots[slot % S3K_SLOT_CNT];
	si->pid = (entry >> 8) & 0xFF;
	si->length = entry & 0xFF;
}

static proc_t *sched_fetch(uint64_t slot)
{
	proc_t *proc = NULL;
	slot_info_t si;
	// Get time slot information
	slot_info_get(slot, &si);

	// If length = 0, then slice is deleted.
	if (si.length == 0)
		return NULL;

	proc = proc_get(si.pid);

	// Try to acquire the process.
	if (!proc_acquire(proc)) {
		return NULL;
	}

	// Get the process.
	proc->timeout = (slot + si.length) * S3K_SLOT_LEN;
	return proc;
}

proc_t *sched(void)
{
	// Hart ID
	uint64_t hart = csrr_mhartid();
	// Time slot
	uint64_t slot;
	// Process to schedule
	proc_t *proc;
	rtc_timeout_set(hart, (uint64_t)-1);

	do {
		slot = rtc_time_get() / S3K_SLOT_LEN;

		// Try schedule process
		proc = sched_fetch(slot);
	} while (!proc);
	rtc_timeout_set(hart, proc->timeout);
	return proc;
}
