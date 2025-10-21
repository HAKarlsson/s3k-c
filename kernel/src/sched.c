/* See LICENSE file for copyright and license details. */

#include "sched.h"

#include "csr.h"
#include "kprint.h"
#include "libkernel.h"
#include "rtc.h"

extern kstate_t ks;

void Sched_update(u64 pid, u64 end, u64 from, u64 to)
{
	for (u64 i = from; i < to; i++) {
		ks.tslots[i] = ((pid << 8) | (end - i));
	}
	return;
}

void Sched_delete(u64 from, u64 to)
{
	u64 mask = 0xFFFFull;
	for (u64 i = from; i < to; ++i)
		ks.tslots[i] &= ~mask;
	return;
}

proc_t *sched(void)
{
	// Process to schedule
	Sched_fetch();
	return proc_get(ks.next_pid);
}
