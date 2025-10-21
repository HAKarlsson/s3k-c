#include "kprint.h"
#include "kstate.h"
#include "libkernel.h"
#include "macro.h"
#include "proc.h"
#include "types.h"
#include "uart/ns16550a.h"

extern char __uart[];

extern kstate_t ks;

void kernel_init(void)
{
	kputs("# Kernel init");

	cap_t init_caps[] = {
	    Util_cap_mk_pmp(Util_pmp_napot_encode(0x80010000, 0x10000),
			    MEM_RWX),
	    Util_cap_mk_memory(0x80010000, 0x80100000, MEM_RWX),
	    Util_cap_mk_memory(0x10000000, 0x10001000, MEM_RW),
	    Util_cap_mk_time(0, S3K_SLOT_CNT),
	    Util_cap_mk_monitor(0, S3K_PROC_CNT),
	    Util_cap_mk_channel(0, S3K_CHAN_CNT),
	};

	kstate_init(init_caps, ARRAY_SIZE(init_caps));

	proc_init(0x80010000);

	Pmp_load(Cap_ops_cap_idx_from_pid(0, 0), 0);
}

void kputc(char c)
{
	uart_ns16550a_putchar(c, __uart);
}
