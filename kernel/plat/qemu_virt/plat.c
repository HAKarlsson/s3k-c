#include "cap/pmp.h"
#include "cap/table.h"
#include "cap/util.h"
#include "kinit.h"
#include "libkernel.h"
#include "pmp.h"
#include "proc.h"
#include "sched.h"
#include "types.h"
#include "uart/ns16550a.h"

extern char __uart[];

extern struct Kernel_state ks;

// static unsigned long long _vreg[8];

void kernel_init(void)
{
	kputs("# Kernel init");

	kinit_vregs();

	kputs("# Initialize capability table");

	cap_t init_caps[5];

	cap_mk_pmp(&init_caps[0], pmp_napot_encode(0x80010000, 0x10000),
		   MEM_RWX);
	cap_mk_memory(&init_caps[1], 0x80010000, 0x80100000, MEM_RWX);
	cap_mk_memory(&init_caps[2], 0x10000000, 0x10010000, MEM_RW);
	cap_mk_time(&init_caps[3], 0, S3K_SLOT_CNT);
	cap_mk_monitor(&init_caps[4], 0, S3K_PROC_CNT);
	kinit_ctable(init_caps, ARRAY_SIZE(init_caps));

	kputs("# Initialize scheduler");
	sched_init();

	kputs("# Initialize processes");
	proc_init(0x80010000);

	kputs("# Load INIT PMP");
	Pmp_load(&ks, Cap_ops_cap_idx_from_pid(0, 0), 0);
}

void kputc(char c)
{
	uart_ns16550a_putchar(c, __uart);
}
