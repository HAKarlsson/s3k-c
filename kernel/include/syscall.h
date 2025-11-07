#pragma once

#include "proc.h"

_Static_assert(sizeof(sys_args_t) == 64, "sys_args_t has the wrong size");

proc_t *syscall_handler(proc_t *proc);
proc_t *Profiling_syscall_handler(proc_t *proc);
