#pragma once
#include "types.h"

void cap_mk_time(cap_t *cap, time_slot_t bgn, time_slot_t end);
void cap_mk_memory(cap_t *cap, addr_t bgn, addr_t end, rwx_t rwx);
void cap_mk_pmp(cap_t *cap, napot_t addr, rwx_t rwx);
void cap_mk_monitor(cap_t *cap, pid_t bgn, pid_t end);
void cap_mk_channel(cap_t *cap, chan_t bgn, chan_t end);
void cap_mk_socket(cap_t *cap, chan_t chan, ipc_mode_t mode, ipc_perm_t perm,
		   uint32_t tag);

static inline addr_t tag_block_to_addr(tag_t tag, block_t block)
{
	return ((uint64_t)tag << MAX_BLOCK_SIZE)
	       + ((uint64_t)block << MIN_BLOCK_SIZE);
}

bool cap_is_valid(const cap_t *cap);
void cap_print(const cap_t *cap);
void Cap_print(unsigned long long cap);
