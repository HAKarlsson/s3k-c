#include "cap/util.h"

#include "kprint.h"

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

void cap_print(cap_t cap)
{
	switch (Cap_get_type(cap)) {
	case Cap_CAPTY_NONE:
		kprintf("NONE{}");
		break;
	case Cap_CAPTY_TIME:
		kprintf("TIME{bgn=%d,end=%d,mrk=%d}", Cap_time_get_low(cap),
			Cap_time_get_upp(cap), Cap_time_get_mrk(cap));
		break;

	case Cap_CAPTY_MEMORY:
		u64 bgn = Util_tag_block_to_addr(Cap_memory_get_tag(cap),
						 Cap_memory_get_low(cap));
		u64 end = Util_tag_block_to_addr(Cap_memory_get_tag(cap),
						 Cap_memory_get_upp(cap));
		u64 mrk = Util_tag_block_to_addr(Cap_memory_get_tag(cap),
						 Cap_memory_get_mrk(cap));
		kprintf("MEMORY{bgn=0x%X,end=0x%X,mrk=0x%X,rwx=%s,lck=%x}", bgn,
			end, mrk, rwx2str(Cap_memory_get_rwx(cap)),
			Cap_memory_get_lck(cap));
		break;
	case Cap_CAPTY_PMP:
		word_t pmp_base
		    = Util_pmp_napot_decode_base(Cap_pmp_get_addr(cap));
		word_t pmp_size
		    = Util_pmp_napot_decode_size(Cap_pmp_get_addr(cap));
		kprintf("PMP{bgn=0x%X,end=0x%X,rwx=%s,used=%d,slot=%d}",
			pmp_base, pmp_base + pmp_size,
			rwx2str(Cap_pmp_get_rwx(cap)), Cap_pmp_get_used(cap),
			Cap_pmp_get_slot(cap));
		break;
	case Cap_CAPTY_MONITOR:
		kprintf("MONITOR{bgn=%d,end=%d,mrk=%d}",
			Cap_monitor_get_low(cap), Cap_monitor_get_upp(cap),
			Cap_monitor_get_mrk(cap));
		break;
	case Cap_CAPTY_CHANNEL:
		kprintf("CHANNEL{bgn=%d,end=%d,mrk=%d}",
			Cap_channel_get_low(cap), Cap_channel_get_upp(cap),
			Cap_channel_get_mrk(cap));
		break;
	case Cap_CAPTY_SOCKET:
		kprintf("SOCKET{chan=%d,tag=%d,perm=%d,mode=%d}",
			Cap_socket_get_chan(cap), Cap_socket_get_tag(cap),
			Cap_socket_get_perm(cap), Cap_socket_get_mode(cap));
		break;

	default:
		kprintf("UNKNOWN{raw=0x%X}", cap);
	}
}
