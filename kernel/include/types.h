#pragma once

#include "libkernel.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

typedef u64 word_t;
typedef u64 capty_t;
typedef u64 cap_t;
typedef u64 pid_t;
typedef u64 rwx_t;

typedef enum Error_err_t err_t;

typedef struct Types_kstate kstate_t;

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
