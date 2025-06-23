#include "cap/pmp.h"
#include "cap/table.h"
#include "cap/util.h"
#include "kstate.h"
#include "libkernel.h"
#include "pmp.h"
#include "proc.h"
#include "sched.h"
#include "types.h"
#include "uart/ns16550a.h"

extern char __uart[];

extern struct Types_kstate ks;


void kernel_init(void)
{
	kputs("# Kernel init");

	cap_t init_caps[] = {
	    cap_mk_pmp(pmp_napot_encode(0x80010000, 0x10000), MEM_RWX),
	    cap_mk_memory(0x80010000, 0x80100000, MEM_RWX),
	    cap_mk_memory(0x10000000, 0x10010000, MEM_RW),
	    cap_mk_time(0, S3K_SLOT_CNT),
	    cap_mk_monitor(0, S3K_PROC_CNT),
	    cap_mk_channel(0, S3K_CHAN_CNT),
	};

	kstate_init(init_caps, ARRAY_SIZE(init_caps));

	proc_init(0x80010000);

	Pmp_load(&ks, Cap_ops_cap_idx_from_pid(0, 0), 0);
}

void kputc(char c)
{
	uart_ns16550a_putchar(c, __uart);
}
