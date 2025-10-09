#include "csr.h"

uint64_t csrr_mhartid(void)
{
	uint64_t hartid;
	__asm__ volatile("csrr %0, mhartid" : "=r"(hartid));
	return hartid;
}

uint64_t csrr_mie(void)
{
	uint64_t mie;
	__asm__ volatile("csrr %0, mie" : "=r"(mie));
	return mie;
}

uint64_t csrr_mip(void)
{
	uint64_t mip;
	__asm__ volatile("csrr %0, mip" : "=r"(mip));
	return mip;
}

uint64_t csrr_mcycle(void)
{
	uint64_t mcycle;
	__asm__ volatile("csrr %0, mcycle" : "=r"(mcycle));
	return mcycle;
}

void csrw_pmpaddr0(word_t addr)
{
	__asm__ volatile("csrw pmpaddr0, %0" : : "r"(addr));
}

void csrw_pmpaddr1(word_t addr)
{
	__asm__ volatile("csrw pmpaddr1, %0" : : "r"(addr));
}

void csrw_pmpaddr2(word_t addr)
{
	__asm__ volatile("csrw pmpaddr2, %0" : : "r"(addr));
}

void csrw_pmpaddr3(word_t addr)
{
	__asm__ volatile("csrw pmpaddr3, %0" : : "r"(addr));
}

void csrw_pmpaddr4(word_t addr)
{
	__asm__ volatile("csrw pmpaddr4, %0" : : "r"(addr));
}

void csrw_pmpaddr5(word_t addr)
{
	__asm__ volatile("csrw pmpaddr5, %0" : : "r"(addr));
}

void csrw_pmpaddr6(word_t addr)
{
	__asm__ volatile("csrw pmpaddr6, %0" : : "r"(addr));
}

void csrw_pmpaddr7(word_t addr)
{
	__asm__ volatile("csrw pmpaddr7, %0" : : "r"(addr));
}

void csrw_pmpcfg0(word_t cfg)
{
	__asm__ volatile("csrw pmpcfg0, %0" : : "r"(cfg));
}

void csrw_pmpcfg1(word_t cfg)
{
	__asm__ volatile("csrw pmpcfg1, %0" : : "r"(cfg));
}
