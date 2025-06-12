#include "cap/util.h"
#include "csr.h"
#include "libkernel.h"
#include "machine.h"
#include "pmp.h"
#include "proc.h"
#include "rtc.h"
#include "sched.h"

#include <stdio.h>

static uint64_t __mtime[1];
static uint64_t __mtimecmp[8];
static word_t pmpaddr[8];
static word_t pmpcfg[2];

word_t csrr_mhartid(void)
{
	return 0;
}

word_t csrr_mie(void)
{
	return 128;
}

word_t csrr_mip(void)
{
	if (rtc_time_get() > rtc_timeout_get(0)) {
		return 128; // Simulate an interrupt
	}
	return 0;
}

void csrw_pmpaddr0(word_t addr)
{
	pmpaddr[0] = addr;
}

void csrw_pmpaddr1(word_t addr)
{
	pmpaddr[1] = addr;
}

void csrw_pmpaddr2(word_t addr)
{
	pmpaddr[2] = addr;
}

void csrw_pmpaddr3(word_t addr)
{
	pmpaddr[3] = addr;
}

void csrw_pmpaddr4(word_t addr)
{
	pmpaddr[4] = addr;
}

void csrw_pmpaddr5(word_t addr)
{
	pmpaddr[5] = addr;
}

void csrw_pmpaddr6(word_t addr)
{
	pmpaddr[6] = addr;
}

void csrw_pmpaddr7(word_t addr)
{
	pmpaddr[7] = addr;
}

void csrw_pmpcfg0(word_t cfg)
{
	pmpcfg[0] = cfg;
}

void csrw_pmpcfg1(word_t cfg)
{
	pmpcfg[1] = cfg;
}

void kputc(char c)
{
}

uint64_t rtc_time_get(void)
{
	return __mtime[0]++;
}

void rtc_time_set(uint64_t time)
{
	__mtime[0] = time;
}

uint64_t rtc_timeout_get(uint64_t hart)
{
	return __mtimecmp[hart];
}

void rtc_timeout_set(uint64_t hart, uint64_t time)
{
	__mtimecmp[hart] = time;
}

u64 Machine_time_read(struct Machine_state *mc)
{
	return rtc_time_get();
}

u64 Machine_timeout_read(struct Machine_state *mc, u64 hart)
{
	return rtc_timeout_get(hart);
}

u64 Machine_csrr_mhartid(struct Machine_state *mc)
{
	return csrr_mhartid();
}
