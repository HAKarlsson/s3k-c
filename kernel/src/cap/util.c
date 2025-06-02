#include "cap/util.h"

#include "kassert.h"
#include "libkernel.h"
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
		kprintf("MEMORY{bgn=0x%X,end=0x%X,mrk=0x%X,rwx=%d,lck=%x}", bgn,
			end, mrk, cap->mem.rwx, cap->mem.lck);
	} break;
	case CAPTY_PMP: {
		word_t pmp_base = pmp_napot_decode_base(cap->pmp.addr);
		word_t pmp_size = pmp_napot_decode_size(cap->pmp.addr);
		kprintf("PMP{bgn=0x%X,end=0x%X,rwx=%d,used=%d,slot=%d}",
			pmp_base, pmp_base + pmp_size, cap->pmp.rwx,
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

void Cap_print(u64 cap)
{
	switch (Cap_get_type(cap)) {
	case CAPTY_NONE:
		kprintf("NONE{}");
		break;
	case CAPTY_TIME:
		kprintf("TIME{bgn=%U,end=%U,mrk=%U}", Cap_time_get_bgn(cap),
			Cap_time_get_end(cap), Cap_time_get_mrk(cap));
		break;
	case CAPTY_MEMORY: {
		uint64_t bgn = Cap_ops_tag_block_to_addr(
		    Cap_memory_get_tag(cap), Cap_memory_get_bgn(cap));
		uint64_t end = Cap_ops_tag_block_to_addr(
		    Cap_memory_get_tag(cap), Cap_memory_get_end(cap));
		uint64_t mrk = Cap_ops_tag_block_to_addr(
		    Cap_memory_get_tag(cap), Cap_memory_get_mrk(cap));
		kprintf("MEMORY{bgn=0x%X,end=0x%X,mrk=0x%X,rwx=%d,lck=%x}", bgn,
			end, mrk, Cap_memory_get_rwx(cap),
			Cap_memory_get_lck(cap));
	} break;
	case CAPTY_PMP: {
		word_t pmp_base = pmp_napot_decode_base(Cap_pmp_get_addr(cap));
		word_t pmp_size = pmp_napot_decode_size(Cap_pmp_get_addr(cap));
		kprintf("PMP{bgn=0x%X,end=0x%X,rwx=%d,used=%d,slot=%d}",
			pmp_base, pmp_base + pmp_size, Cap_pmp_get_rwx(cap),
			Cap_pmp_get_used(cap), Cap_pmp_get_slot(cap));
	} break;
	case CAPTY_MONITOR:
		kprintf("MONITOR{bgn=%U,end=%U,mrk=%U}",
			Cap_monitor_get_bgn(cap), Cap_monitor_get_end(cap),
			Cap_monitor_get_mrk(cap));
		break;
	case CAPTY_CHANNEL:
		kprintf("CHANNEL{bgn=%U,end=%U,mrk=%U}",
			Cap_channel_get_bgn(cap), Cap_channel_get_end(cap),
			Cap_channel_get_mrk(cap));
		break;
	case CAPTY_SOCKET:
		kprintf("SOCKET{chan=%U,tag=%U,perm=%U,mode=%U}",
			Cap_socket_get_chan(cap), Cap_socket_get_tag(cap),
			Cap_socket_get_perm(cap), Cap_socket_get_mode(cap));
		break;
	default:
		kprintf("UNKNOWN{raw=0x%X}", cap);
	}
}
