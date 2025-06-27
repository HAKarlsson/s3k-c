/* See LICENSE file for copyright and license details. */

#include "sched.h"

#include "csr.h"
#include "libkernel.h"
#include "rtc.h"
#include "kprint.h"

extern kstate_t ks;

kstate_t *Sched_update(kstate_t *ks, u64 pid, u64 end, u64 from, u64 to)
{
	for (u64 i = from; i < to; i++) {
		ks->tslots[i] = ((pid << 8) | (end - i));
	}
	return ks;
}

kstate_t *Sched_delete(kstate_t *ks, u64 from, u64 to)
{
	u64 mask = 0xFFFFull;
	for (u64 i = from; i < to; ++i)
		ks->tslots[i] &= ~mask;
	return ks;
}

proc_t *sched(void)
{
	// Process to schedule
	Sched_fetch(&ks);
	return proc_get(ks.next_pid);
}
