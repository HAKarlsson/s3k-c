#include "cap/util.h"

#include "kassert.h"

void cap_mk_time(cap_t *cap, time_slot_t bgn, time_slot_t end)
{
	KASSERT(bgn < end);
	KASSERT(end <= S3K_SLOT_CNT);
	cap->type = CAPTY_TIME;
	cap->time.bgn = bgn;
	cap->time.mrk = bgn;
	cap->time.end = end;
}

void cap_mk_memory(cap_t *cap, addr_t bgn, addr_t end, rwx_t rwx)
{
	uint64_t tag = bgn >> MAX_BLOCK_SIZE;
	KASSERT(bgn < end);
	KASSERT(end <= (tag + 1) << MAX_BLOCK_SIZE);
	cap->mem.type = CAPTY_MEMORY;
	cap->mem.tag = tag;
	cap->mem.bgn = (bgn - (tag << MAX_BLOCK_SIZE)) >> MIN_BLOCK_SIZE;
	cap->mem.end = (end - (tag << MAX_BLOCK_SIZE)) >> MIN_BLOCK_SIZE;
	cap->mem.mrk = cap->mem.bgn;
	cap->mem.rwx = rwx;
	cap->mem.lck = false;
}

void cap_mk_pmp(cap_t *cap, napot_t addr, rwx_t rwx)
{
	cap->pmp.type = CAPTY_PMP;
	cap->pmp.rwx = rwx;
	cap->pmp.used = 0;
	cap->pmp.slot = 0;
	cap->raw |= addr << 16;
}

void cap_mk_monitor(cap_t *cap, pid_t bgn, pid_t end)
{
	KASSERT(bgn < end);
	KASSERT(end <= S3K_PROC_CNT);
	cap->mon.type = CAPTY_MONITOR;
	cap->mon.bgn = bgn;
	cap->mon.end = end;
	cap->mon.mrk = bgn;
}

void cap_mk_channel(cap_t *cap, chan_t bgn, chan_t end)
{
	KASSERT(bgn < end);
	KASSERT(end <= S3K_CHAN_CNT);
	cap->chan.type = CAPTY_CHANNEL;
	cap->chan.bgn = bgn;
	cap->chan.end = end;
	cap->chan.mrk = bgn;
}

void cap_mk_socket(cap_t *cap, chan_t chan, ipc_mode_t mode, ipc_perm_t perm,
		   uint32_t tag)
{
	cap->sock.type = CAPTY_SOCKET;
	cap->sock.chan = chan;
	cap->sock.mode = mode;
	cap->sock.perm = perm;
	cap->sock.tag = tag;
}

bool cap_is_valid(const cap_t *cap)
{
	switch (cap->type) {
	case CAPTY_TIME:
		return cap->time.bgn < cap->time.end
		       && cap->time.bgn == cap->time.mrk;
	case CAPTY_MEMORY:
		return cap->mem.lck == 0 && cap->mem.bgn < cap->mem.end
		       && cap->mem.mrk == cap->mem.bgn;
	case CAPTY_PMP:
		return cap->pmp.used == 0 && cap->pmp.slot == 0;
	case CAPTY_MONITOR:
		return cap->mon.bgn < cap->mon.end
		       && cap->mon.bgn == cap->mon.mrk;
	case CAPTY_CHANNEL:
		return cap->mem.bgn < cap->mem.end
		       && cap->mem.bgn == cap->mem.mrk;
	case CAPTY_SOCKET:
		return (cap->sock.mode == IPC_YIELD)
		       || (cap->sock.mode == IPC_NOYIELD);
	default:
		return false;
	}
}
