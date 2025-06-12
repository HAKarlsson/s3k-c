#pragma once
#include "types.h"

cap_t cap_mk_time(time_slot_t bgn, time_slot_t end);
cap_t cap_mk_memory(addr_t bgn, addr_t end, rwx_t rwx);
cap_t cap_mk_pmp(napot_t addr, rwx_t rwx);
cap_t cap_mk_monitor(pid_t bgn, pid_t end);
cap_t cap_mk_channel(chan_t bgn, chan_t end);
cap_t cap_mk_socket(chan_t chan, ipc_mode_t mode, ipc_perm_t perm,
		    uint32_t tag);

static inline addr_t tag_block_to_addr(tag_t tag, block_t block)
{
	return ((uint64_t)tag << MAX_BLOCK_SIZE)
	       + ((uint64_t)block << MIN_BLOCK_SIZE);
}

bool cap_is_valid(const cap_t *cap);
void cap_print(const cap_t *cap);
void Cap_print(unsigned long long cap);
