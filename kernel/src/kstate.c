#include "kstate.h"

#include "kprint.h"
#include "libkernel.h"
#include "pmp.h"
#include "proc.h"

struct Types_kstate ks;

// Capabilities
static u64 _cnext[S3K_PROC_CNT * S3K_CAP_CNT];
static u64 _cprev[S3K_PROC_CNT * S3K_CAP_CNT];
static u64 _ctable[S3K_PROC_CNT * S3K_CAP_CNT];

// Virtual registers
static u64 _vregs[8];

// Process table
static u64 _pmpcfg[S3K_PROC_CNT][S3K_PMP_CNT];
static u64 _pmpaddr[S3K_PROC_CNT][S3K_PMP_CNT];
static struct Types_proc_t _procs[S3K_PROC_CNT];
static struct Types_proc_t *_ptable[S3K_PROC_CNT];

// Time slots
static u64 _slots[S3K_SLOT_CNT];

void kstate_init(const cap_t init_caps[], size_t size)
{
	// Setup the kernel state
	ks = (struct Types_kstate){0};
	ks.cnext = _cnext;
	ks.cprev = _cprev;
	ks.ctable = _ctable;
	ks.vregs = _vregs;
	ks.ptable = _ptable;
	ks.tslots = _slots;

	// Zero the capability table
	for (unsigned int i = 0; i < S3K_PROC_CNT * S3K_CAP_CNT; i++) {
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
	for (unsigned int i = 0; i < S3K_PROC_CNT; i++) {
		_procs[i] = (struct Types_proc_t){0};
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
	for (unsigned int i = 0; i < S3K_SLOT_CNT; i++) {
		ks.tslots[i] = 0;
	}

	// Populate the capability table with initial capabilities
	unsigned int prev = 0;
	for (unsigned int i = 0; i < size; i++) {
		if (init_caps[i].type == CAPTY_NONE)
			continue;
		Ctable_insert(&ks, i, init_caps[i].raw, prev);
		prev = i;
		if (init_caps[i].type == Cap_CAPTY_TIME) {
			// Update the time slots when inserting a time capability
			Sched_update(&ks, 0, init_caps[i].time.end,
				     init_caps[i].time.mrk,
				     init_caps[i].time.end);
		}
		kprintf("# init_caps[%d]: %C\n", i, &init_caps[i]);
	}
}
