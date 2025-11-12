#include "cap/pmp.h"
#include "cap/table.h"
#include "cap/util.h"
#include "pmp.h"
#include "proc.h"
#include "sched.h"
#include "types.h"
#include "uart/ns16550a.h"

extern char __uart[];

void kernel_init(void)
{
#ifndef NDEBUG
	kputs("# Kernel init");
#endif
	cap_t init_caps[6];
	cap_mk_pmp(&init_caps[0], pmp_napot_encode(0x80010000, 0x10000),
		   MEM_RWX);
	cap_mk_memory(&init_caps[1], 0x80010000, 0x80100000, MEM_RWX);
	cap_mk_memory(&init_caps[2], 0x10000000, 0x10010000, MEM_RW);
	cap_mk_time(&init_caps[3], 0, S3K_SLOT_CNT);
	cap_mk_monitor(&init_caps[4], 0, S3K_PROC_CNT);
	cap_mk_channel(&init_caps[5], 0, S3K_CHAN_CNT);
	for (int i = 0; i < ARRAY_SIZE(init_caps); ++i) {
#ifndef NDEBUG
		kprintf("# init_caps[%d]: ", i);
#endif
		cap_print(&init_caps[i]);
		kprintf("\n");
	}
	ctable_init(init_caps, ARRAY_SIZE(init_caps));

	sched_init();
	proc_init(0x80010000);
	cap_pmp_load(ctable_get(0, 0), 0);
}

void kputc(char c)
{
	uart_ns16550a_putchar(c, __uart);
}
