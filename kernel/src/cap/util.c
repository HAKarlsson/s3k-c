#include "cap/util.h"

#include "kassert.h"
#include "libkernel.h"
#include "pmp.h"

cap_t cap_mk_time(time_slot_t bgn, time_slot_t end)
{
	KASSERT(bgn < end);
	KASSERT(end <= S3K_SLOT_CNT);
	return (cap_t){
	    .time = {
		     .type = CAPTY_TIME,
		     ._padding = 0, // Ensure padding is zero
		.bgn = bgn,
		     .mrk = bgn,
		     .end = end,
		     }
	     };
}

cap_t cap_mk_memory(addr_t bgn, addr_t end, rwx_t rwx)
{
	uint64_t tag = bgn >> MAX_BLOCK_SIZE;
	KASSERT(bgn < end);
	KASSERT(end <= (tag + 1) << MAX_BLOCK_SIZE);
	return (cap_t){
	    .mem = {
		    .type = CAPTY_MEMORY,
		    .rwx = rwx,
		    .lck = false,
		    .tag = tag,
		    .bgn = (bgn - (tag << MAX_BLOCK_SIZE)) >> MIN_BLOCK_SIZE,
		    .mrk = (bgn - (tag << MAX_BLOCK_SIZE)) >> MIN_BLOCK_SIZE,
		    .end = (end - (tag << MAX_BLOCK_SIZE)) >> MIN_BLOCK_SIZE,
		    }
	     };
}

cap_t cap_mk_pmp(napot_t addr, rwx_t rwx)
{
	return (cap_t){
	    .pmp = {
		    .type = CAPTY_PMP,
		    .rwx = rwx,
		    .used = false,
		    .slot = 0,
		    .addr = addr,
		    }
	     };
}

cap_t cap_mk_monitor(pid_t bgn, pid_t end)
{
	KASSERT(bgn < end);
	KASSERT(end <= S3K_PROC_CNT);
	return (cap_t){
	    .mon = {
		    .type = CAPTY_MONITOR,
		    ._padding = 0, // Ensure padding is zero
		.bgn = bgn,
		    .mrk = bgn,
		    .end = end,
		    }
	     };
}

cap_t cap_mk_channel(chan_t bgn, chan_t end)
{
	KASSERT(bgn < end);
	KASSERT(end <= S3K_CHAN_CNT);
	return (cap_t){
	    .chan = {
		     .type = CAPTY_CHANNEL,
		     ._padding = 0, // Ensure padding is zero
		.bgn = bgn,
		     .mrk = bgn,
		     .end = end,
		     }
	     };
}

cap_t cap_mk_socket(chan_t chan, ipc_mode_t mode, ipc_perm_t perm, uint32_t tag)
{
	return (cap_t){
	    .sock = {
		     .type = CAPTY_SOCKET,
		     .mode = mode,
		     .perm = perm,
		     .chan = chan,
		     .tag = tag,
		     }
	     };
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
	case MEM_WX:
		return "-WX";
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
			pmp_base, pmp_base + pmp_size, rwx2str(cap->pmp.rwx),
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

void Cap_print(u64 cap_raw)
{
	cap_t cap = {.raw = cap_raw};
	cap_print(&cap);
}
