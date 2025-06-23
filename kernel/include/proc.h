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

typedef struct Types_proc_t proc_t;
typedef word_t proc_state_t;

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

bool proc_acquire(proc_t *proc);
void proc_release(proc_t *proc);
proc_t *proc_pmp_sync(proc_t *proc);
