#include "csr.h"
#include "rtc.h"

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
