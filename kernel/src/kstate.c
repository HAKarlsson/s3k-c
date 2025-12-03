#include "kstate.h"

#include "kprint.h"
#include "libkernel.h"
#include "macro.h"
#include "sched.h"

kstate_t ks;
kstate_t *const Kernel_ks = &ks;

void kstate_init(const cap_t init_caps[], size_t size)
{
#ifndef NDEBUG
	kputs("# Kernel init");
#endif

	// Setup the kernel state
	ks.next_pid = Proc_pid_null; // Initialize next PID to NULL

	// Zero the capability table
	for (unsigned int i = 0; i < ARRAY_SIZE(ks.ctable); i++) {
		ks.ctable[i].next = 0;
		ks.ctable[i].prev = 0;
		ks.ctable[i].cap = 0;
	}

	// Zero the virtual registers
	for (unsigned int i = 0; i < 8; i++) {
		ks.vregs[i] = 0;
	}
	ks.errcode = 0;

	// Zero and intialize the process table
	for (unsigned int i = 0; i < ARRAY_SIZE(ks.ptable); i++) {
		for (unsigned int j = 0; j < S3K_PMP_CNT; j++) {
			ks.ptable[i].pmpcfg = 0;
			ks.ptable[i].pmpaddr[j] = 0;
		}
		ks.ptable[i].state = Proc_psf_suspended;
		ks.ptable[i].pid = i;
	}

	// Zero the time slots
	for (unsigned int i = 0; i < ARRAY_SIZE(ks.tslots); i++) {
		ks.tslots[i] = 0;
	}

	for (unsigned int i = 0; i < ARRAY_SIZE(ks.channels); i++) {
		ks.channels[i].server = 0;
		ks.channels[i].client = 0;
		ks.channels[i].buf_idx = 0;
	}

	// Populate the capability table with initial capabilities
	unsigned int prev = 0;
	for (unsigned int i = 0; i < size; i++) {
		cap_t cap = init_caps[i];
		if (Cap_get_type(cap) == Cap_Capty_none)
			continue;
		Ctable_insert(i, cap, prev);
		prev = i;
		if (Cap_get_type(cap) == Cap_Capty_time) {
			// Update the time slots when inserting a time capability
			Sched_update(0, Cap_time_get_upp(cap),
				     Cap_time_get_mrk(cap),
				     Cap_time_get_upp(cap));
		}
#ifndef NDEBUG
		kprintf("# init_caps[%d]: %C\n", i, cap);
#endif
	}
}
