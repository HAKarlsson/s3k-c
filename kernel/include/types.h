#pragma once

#include "bint.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

typedef u64 word_t;
typedef u64 capty_t;
typedef u64 cap_t;
typedef u64 pid_t;
typedef u64 rwx_t;

typedef i64 err_t;

typedef enum {
	// Basic info & registers
	SYSCALL_GET_INFO,
	SYSCALL_REG_READ,
	SYSCALL_REG_WRITE,
	SYSCALL_SYNC,
	SYSCALL_SLEEP,

	// Capability management calls
	SYSCALL_CAP_READ,
	SYSCALL_CAP_MOVE,
	SYSCALL_CAP_DELETE,
	SYSCALL_CAP_DERIVE,
	SYSCALL_CAP_REVOKE,

	// Pmp calls
	SYSCALL_PMP_LOAD,
	SYSCALL_PMP_UNLOAD,

	// Monitor calls
	SYSCALL_MON_SUSPEND,
	SYSCALL_MON_RESUME,
	SYSCALL_MON_STATE_GET,
	SYSCALL_MON_YIELD,
	SYSCALL_MON_REG_READ,
	SYSCALL_MON_REG_WRITE,
	SYSCALL_MON_CAP_READ,
	SYSCALL_MON_CAP_MOVE,
	SYSCALL_MON_PMP_LOAD,
	SYSCALL_MON_PMP_UNLOAD,

	// Socket calls
	SYSCALL_SOCK_SEND,
	SYSCALL_SOCK_RECV,
	SYSCALL_SOCK_SENDRECV,
} syscall_t;

typedef union {
	struct {
		word_t a0, a1, a2, a3, a4, a5, a6, a7;
	};

	struct {
		word_t info;
	} get_info;

	struct {
		word_t reg;
	} reg_read;

	struct {
		word_t reg;
		word_t val;
	} reg_write;

	struct {
		word_t full;
	} sync;

	struct {
		word_t time;
	} sleep;

	struct {
		word_t idx;
	} cap_read;

	struct {
		word_t src_idx;
		word_t dst_idx;
	} cap_move;

	struct {
		word_t idx;
	} cap_delete;

	struct {
		word_t idx;
	} cap_revoke;

	struct {
		word_t src_idx;
		word_t dst_idx;
		word_t cap_raw;
	} cap_derive;

	struct {
		word_t idx;
		word_t slot;
	} pmp_load;

	struct {
		word_t idx;
	} pmp_unload;

	struct {
		word_t mon_idx;
		word_t pid;
	} mon_state;

	struct {
		word_t mon_idx;
		word_t pid;
		word_t reg;
	} mon_reg_read;

	struct {
		word_t mon_idx;
		word_t pid;
		word_t reg;
		word_t val;
	} mon_reg_write;

	struct {
		word_t mon_idx;
		word_t pid;
		word_t idx;
	} mon_cap_read;

	struct {
		word_t mon_idx;
		word_t src_pid;
		word_t src_idx;
		word_t dst_pid;
		word_t dst_idx;
	} mon_cap_move;

	struct {
		word_t mon_idx;
		word_t pid;
		word_t idx;
		word_t slot;
	} mon_pmp_load;

	struct {
		word_t mon_idx;
		word_t pid;
		word_t idx;
	} mon_pmp_unload;

	struct {
		word_t sock_idx;
		word_t cap_idx;
		word_t send_cap;
		word_t data[4];
	} sock;
} sys_args_t;

typedef enum {
	MEM_NONE = 0,
	MEM_R = 0x1,
	MEM_W = 0x2,
	MEM_X = 0x4,
	MEM_RW = MEM_R | MEM_W,
	MEM_RX = MEM_R | MEM_X,
	MEM_WX = MEM_W | MEM_X,
	MEM_RWX = MEM_R | MEM_W | MEM_X,
} mem_perm_t;

_Static_assert(sizeof(cap_t *) == 8, "cap_mem has the wrong size");
