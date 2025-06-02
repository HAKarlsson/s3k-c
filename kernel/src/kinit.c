#include "kinit.h"

#include "kprint.h"
#include "libkernel.h"
#include "pmp.h"
#include "proc.h"

struct Kernel_state ks;

// Capabilities
static u64 _cnext[S3K_PROC_CNT * S3K_CAP_CNT];
static u64 _cprev[S3K_PROC_CNT * S3K_CAP_CNT];
static u64 _ctable[S3K_PROC_CNT * S3K_CAP_CNT];

// Virtual registers
static u64 _vregs[8];

u64 convert_cap(cap_t cap)
{
	switch (cap.type) {
	case CAPTY_MEMORY:
		return Cap_mk_memory(cap.mem.rwx, cap.mem.lck, cap.mem.tag,
				     cap.mem.bgn, cap.mem.mrk, cap.mem.end);
	case CAPTY_PMP:
		return Cap_mk_pmp(cap.pmp.rwx, cap.pmp.used, cap.pmp.slot,
				  cap.pmp.addr);
	case CAPTY_TIME:
		return Cap_mk_time(cap.time.bgn, cap.time.mrk, cap.time.end);
	case CAPTY_MONITOR:
		return Cap_mk_monitor(cap.time.bgn, cap.time.mrk, cap.time.end);
	default:
		return Cap_mk_none();
	}
}

void kinit_ctable(const cap_t init_caps[], size_t size)
{
	ks.cnext = _cnext;
	ks.cprev = _cprev;
	ks.ctable = _ctable;
	for (unsigned int i = 0; i < size; i++) {
		if (init_caps[i].type == CAPTY_NONE)
			continue;
		u64 c = convert_cap(init_caps[i]);
		Ctable_insert(&ks, i, c, 0);
		kprintf("# init_caps[%d]: %C\n", i, &init_caps[i]);
	}
}

void kinit_vregs(void)
{
	ks.vregs = _vregs;
}