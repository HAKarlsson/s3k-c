#include "cap/util.h"
#

#include "kassert.h"
#include "pmp.h"

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
	cap->pmp.addr = addr;
	cap->pmp.rwx = rwx;
	cap->pmp.used = 0;
	cap->pmp.slot = 0;
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

const char *rwx2str(rwx_t rwx)
{
	switch (rwx) {
	case MEM_RWX:
		return "RWX";
	case MEM_RW:
		return "RW-";
	case MEM_RX:
		return "R-X";
	case MEM_R:
		return "R--";
	case MEM_W:
		return "-W-";
	case MEM_X:
		return "--X";
	default:
		return "---";
	}
}

void cap_print(const cap_t *cap)
{
	switch (cap->type) {
	case CAPTY_NONE:
		kprintf("NONE{}");
		break;
	case CAPTY_TIME:
		kprintf("TIME{bgn=%d,end=%d,mrk=%d}", cap->time.bgn,
			cap->time.end, cap->time.mrk);
		break;
	case CAPTY_MEMORY: {
		uint64_t bgn = tag_block_to_addr(cap->mem.tag, cap->mem.bgn);
		uint64_t end = tag_block_to_addr(cap->mem.tag, cap->mem.end);
		uint64_t mrk = tag_block_to_addr(cap->mem.tag, cap->mem.mrk);
		kprintf("MEMORY{bgn=0x%X,end=0x%X,mrk=0x%X,rwx=%s,lck=%x}", bgn,
			end, mrk, rwx2str(cap->mem.rwx), cap->mem.lck);
	} break;
	case CAPTY_PMP: {
		word_t pmp_base = pmp_napot_decode_base(cap->pmp.addr);
		word_t pmp_size = pmp_napot_decode_size(cap->pmp.addr);
		kprintf("PMP{bgn=0x%X,end=0x%X,rwx=%s,used=%d,slot=%d}",
			pmp_base, pmp_base + pmp_size, rwx2str(cap->mem.rwx),
			cap->pmp.used, cap->pmp.slot);
	} break;
	case CAPTY_MONITOR:
		kprintf("MONITOR{bgn=%d,end=%d,mrk=%d}", cap->mon.bgn,
			cap->mon.end, cap->mon.mrk);
		break;
	case CAPTY_CHANNEL:
		kprintf("CHANNEL{bgn=%d,end=%d,mrk=%d}", cap->chan.bgn,
			cap->chan.end, cap->chan.mrk);
		break;
	case CAPTY_SOCKET:
		kprintf("SOCKET{chan=%d,tag=%d,perm=%d,mode=%d}",
			cap->sock.chan, cap->sock.tag, cap->sock.perm,
			cap->sock.mode);
		break;
	default:
		kprintf("UNKNOWN{raw=0x%X}", cap->raw);
	}
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
