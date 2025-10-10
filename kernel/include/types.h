#pragma once

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Min logarithmic size of a memory slice
#define MIN_BLOCK_SIZE 12

// Max logarithmic size of a memory slice
#define MAX_BLOCK_SIZE 27

typedef unsigned long word_t;

typedef uint64_t napot_t;
typedef uint64_t addr_t;
typedef uint16_t block_t;
typedef uint16_t chan_t;
typedef uint16_t time_slot_t;
typedef uint16_t pid_t;
typedef uint16_t cidx_t;
typedef uint8_t hart_t;
typedef uint8_t tag_t;
typedef uint8_t rwx_t;
typedef uint8_t pmp_slot_t;

typedef enum {
	SUCCESS = 0,
	ERR_EMPTY,
	ERR_SRC_EMPTY,
	ERR_DST_OCCUPIED,
	ERR_INVALID_INDEX,
	ERR_INVALID_DERIVATION,
	ERR_INVALID_MONITOR,
	ERR_INVALID_PID,
	ERR_INVALID_STATE,
	ERR_INVALID_PMP,
	ERR_INVALID_SLOT,
	ERR_INVALID_SOCKET,
	ERR_INVALID_SYSCALL,
	ERR_INVALID_REGISTER,
	ERR_INVALID_CAPABILITY,
	ERR_NO_RECEIVER,
	ERR_PREEMPTED,
	ERR_TIMEOUT,
	ERR_SUSPENDED,
	CONTINUE = -1, // Special for revocation
} err_t;

typedef enum {
	// Basic Info & Registers
	SYSCALL_GET_INFO,  // Retrieve basic system information
	SYSCALL_REG_READ,  // Set the value of a specific register
	SYSCALL_REG_WRITE, // Get the value of a specific register
	SYSCALL_SYNC,	   // Synchronize with capabilities/scheduling
	SYSCALL_SLEEP,

	// Capability Management
	SYSCALL_CAP_READ,   // Read the properties of a capability
	SYSCALL_CAP_MOVE,   // Move a capability to a different slot
	SYSCALL_CAP_DELETE, // Remove a capability from the system
	SYSCALL_CAP_DERIVE, // Derive a new capability from an existing one
	SYSCALL_CAP_REVOKE, // Revoke a derived capabilities

	// PMP
	SYSCALL_PMP_LOAD,
	SYSCALL_PMP_UNLOAD,

	// Monitor
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

	// Socket
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
	MEM_RWX = MEM_R | MEM_W | MEM_X,
} mem_perm_t;

// IPC Modes
typedef enum {
	IPC_NOYIELD = 0x0, // Non-Yielding Synchronous
	IPC_YIELD = 0x1,   // Yielding Synchronous
} ipc_mode_t;

// IPC Permissions
typedef enum {
	IPC_SDATA = 0x1, // Server can send data
	IPC_SCAP = 0x2,	 // Server can send capabilities
	IPC_CDATA = 0x4, // Client can send data
	IPC_CCAP = 0x8,	 // Client can send capabilities
} ipc_perm_t;

// Capability types
typedef enum capty {
	CAPTY_NONE = 0,	   ///< No capability.
	CAPTY_TIME = 1,	   ///< Time Slice capability.
	CAPTY_MEMORY = 2,  ///< Memory Slice capability.
	CAPTY_PMP = 3,	   ///< PMP Frame capability.
	CAPTY_MONITOR = 4, ///< Monitor capability.
	CAPTY_CHANNEL = 5, ///< IPC Channel capability.
	CAPTY_SOCKET = 6,  ///< IPC Socket capability.
	CAPTY_COUNT	   ///< Number of capability types
} capty_t;

/// Capability description
typedef union cap {
	capty_t type : 4;

	uint64_t raw;

	struct {
		capty_t type : 4;
		uint16_t _padding : 12;
		time_slot_t bgn;
		time_slot_t mrk;
		time_slot_t end;
	} time;

	struct {
		capty_t type : 4;
		rwx_t rwx : 3;
		bool lck : 1;
		tag_t tag : 8;
		block_t bgn : 16;
		block_t mrk : 16;
		block_t end : 16;
	} mem;

	struct {
		capty_t type : 4;
		rwx_t rwx : 3;
		bool used : 1;
		pmp_slot_t slot;
		uint32_t addr_lo;
		uint16_t addr_hi;
	} pmp;

	struct {
		capty_t type : 4;
		uint16_t _padding : 12;
		pid_t bgn : 16;
		pid_t mrk : 16;
		pid_t end : 16;
	} mon;

	struct {
		capty_t type : 4;
		uint16_t _padding : 12;
		chan_t bgn;
		chan_t mrk;
		chan_t end;
	} chan;

	struct {
		capty_t type : 4;
		ipc_mode_t mode : 4;
		ipc_perm_t perm : 8;
		chan_t chan;
		uint32_t tag;
	} sock;
} cap_t;

_Static_assert(sizeof(cap_t *) == 8, "cap_mem has the wrong size");
