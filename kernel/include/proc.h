#pragma once
/**
 * @file proc.h
 * @brief Defines the process control block and its associated functions.
 *
 * This file contains the definition of the `proc_t` data structure, which
 * represents a process control block (PCB) in the operating system. It also
 * contains the declarations of functions for manipulating the PCB.
 *
 * @copyright MIT License
 */

#include "libkernel.h"
#include "types.h"

typedef struct Proc_proc_t proc_t;
typedef word_t proc_state_t;

typedef enum {
	PSF_BUSY = 1,
	PSF_BLOCKED = 2,
	PSF_SUSPENDED = 4,
} proc_state_flags_t;

/**
 * Initializes all processes in the system.
 *
 * @note This function should be called only once during system startup.
 */
void proc_init(word_t payload);

/**
 * @brief Gets the process corresponding to a given process ID.
 *
 * @param pid The process ID to look for.
 * @return A pointer to the process corresponding to the given PID.
 */
proc_t *proc_get(pid_t pid);

proc_state_t proc_get_state(proc_t *proc);

bool proc_acquire(proc_t *proc);
void proc_release(proc_t *proc);
void proc_suspend(proc_t *proc);
void proc_resume(proc_t *proc);
void proc_ipc_wait(proc_t *proc, chan_t chan);
bool proc_ipc_acquire(proc_t *proc, chan_t chan);
bool proc_is_suspended(proc_t *proc);

bool proc_pmp_avail(proc_t *proc, pmp_slot_t slot);
void proc_pmp_load(proc_t *proc, pmp_slot_t slot, rwx_t cfg, napot_t addr);
void proc_pmp_unload(proc_t *proc, pmp_slot_t slot);
proc_t *proc_pmp_sync(proc_t *proc);
