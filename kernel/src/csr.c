#include "csr.h"

u64 csrr_mhartid(void)
{
	u64 hartid;
	__asm__ volatile("csrr %0, mhartid" : "=r"(hartid));
	return hartid;
}

u64 Machine_csrr_mhartid(struct Machine_state *mc)
{
	return csrr_mhartid();
}

u64 csrr_mie(void)
{
	u64 mie;
	__asm__ volatile("csrr %0, mie" : "=r"(mie));
	return mie;
}

u64 csrr_mip(void)
{
	u64 mip;
	__asm__ volatile("csrr %0, mip" : "=r"(mip));
	return mip;
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
