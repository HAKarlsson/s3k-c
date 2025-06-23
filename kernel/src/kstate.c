#include "kstate.h"

#include "kprint.h"
#include "libkernel.h"
#include "macro.h"
#include "proc.h"

struct Types_kstate ks;

// Capabilities
#define TOTAL_CAP_CNT (S3K_PROC_CNT * S3K_CAP_CNT)
static u64 _cnext[TOTAL_CAP_CNT];
static u64 _cprev[TOTAL_CAP_CNT];
static u64 _ctable[TOTAL_CAP_CNT];

// Virtual registers
static u64 _vregs[8];

// Process table
static u64 _pmpcfg[S3K_PROC_CNT][S3K_PMP_CNT];
static u64 _pmpaddr[S3K_PROC_CNT][S3K_PMP_CNT];
static struct Types_proc_t _procs[S3K_PROC_CNT];
static struct Types_proc_t *_ptable[S3K_PROC_CNT];

// Time slots
static u64 _tslots[S3K_SLOT_CNT];

// Channels
static struct Types_channel _channels[S3K_CHAN_CNT];
static struct Types_channel *_pchannels[S3K_CHAN_CNT];

void kstate_init(const cap_t init_caps[], size_t size)
{
	// Setup the kernel state
	ks.cnext = _cnext;
	ks.cprev = _cprev;
	ks.ctable = _ctable;
	ks.vregs = _vregs;
	ks.ptable = _ptable;
	ks.tslots = _tslots;
	ks.channels = _pchannels;

	// Zero the capability table
	for (unsigned int i = 0; i < ARRAY_SIZE(_ctable); i++) {
		ks.cnext[i] = 0;
		ks.cprev[i] = 0;
		ks.ctable[i] = 0;
	}

	// Zero the virtual registers
	for (unsigned int i = 0; i < 8; i++) {
		ks.vregs[i] = 0;
	}
	ks.errcode = 0;

	// Zero and intialize the process table
	for (unsigned int i = 0; i < ARRAY_SIZE(_ptable); i++) {
		_ptable[i] = &_procs[i];
		for (unsigned int j = 0; j < S3K_PMP_CNT; j++) {
			_pmpcfg[i][j] = 0;
			_pmpaddr[i][j] = 0;
		}
		_procs[i].pmpaddr = _pmpaddr[i];
		_procs[i].pmpcfg = _pmpcfg[i];
		ks.ptable[i]->state = Proc_PSF_SUSPENDED;
		ks.ptable[i]->pid = i;
	}

	// Zero the time slots
	for (unsigned int i = 0; i < ARRAY_SIZE(_tslots); i++) {
		ks.tslots[i] = 0;
	}

	for (unsigned int i = 0; i < ARRAY_SIZE(_channels); i++) {
		ks.channels[i] = &_channels[i];
		ks.channels[i]->server = 0;
		ks.channels[i]->client = 0;
		ks.channels[i]->buf_idx = 0;
	}

	// Populate the capability table with initial capabilities
	unsigned int prev = 0;
	for (unsigned int i = 0; i < size; i++) {
		cap_t cap = init_caps[i];
		if (Cap_get_type(cap) == Cap_CAPTY_NONE)
			continue;
		Ctable_insert(&ks, i, cap, prev);
		prev = i;
		if (Cap_get_type(cap) == Cap_CAPTY_TIME) {
			// Update the time slots when inserting a time capability
			Sched_update(&ks, 0, Cap_time_get_end(cap),
				     Cap_time_get_mrk(cap),
				     Cap_time_get_end(cap));
		}
		kprintf("# init_caps[%d]: %C\n", i, cap);
	}
}
