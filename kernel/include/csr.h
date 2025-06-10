/**
 * @file csr.h
 * @brief Interact with control and status registers.
 * @copyright MIT License
 * @author Henrik Karlsson (henrik10@kth.se)
 */
#pragma once

#ifndef __ASSEMBLER__
#include <types.h>
#include "libkernel.h"
#endif

/* Machine CSR constants */
#define MIP_MSIP 0x8
#define MIE_MSIE 0x8
#define MIP_MTIP 0x80
#define MIE_MTIE 0x80
#define MCAUSE_USER_ECALL 0x8
#define MSTATUS_MIE 0x8

#ifndef __ASSEMBLER__
word_t csrr_mhartid(void);
word_t csrr_mie(void);
word_t csrr_mip(void);

void csrw_pmpaddr0(word_t addr);
void csrw_pmpaddr1(word_t addr);
void csrw_pmpaddr2(word_t addr);
void csrw_pmpaddr3(word_t addr);
void csrw_pmpaddr4(word_t addr);
void csrw_pmpaddr5(word_t addr);
void csrw_pmpaddr6(word_t addr);
void csrw_pmpaddr7(word_t addr);
void csrw_pmpcfg0(word_t cfg);
void csrw_pmpcfg1(word_t cfg);
#endif
