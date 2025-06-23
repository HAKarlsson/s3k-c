/* See LICENSE file for copyright and license details. */
#include "syscall.h"

#include "cap/ops.h"
#include "cap/util.h"
#include "csr.h"
#include "kprint.h"
#include "libkernel.h"
#include "rtc.h"
#include "sched.h"
#include "trap.h"
#include "types.h"

#include <stdbool.h>

extern struct Types_kstate ks;

static inline proc_t *next_proc()
{
	u64 next_pid = Vreg_read(&ks, 0);
	proc_t *next = next_pid == Proc_NULL ? NULL : ks.ptable[next_pid];
	return next;
}

proc_t *get_info(proc_t *const p, const sys_args_t *args)
{
	Syscall_get_info(&ks, p->pid, args->get_info.info);
	return p;
}

proc_t *reg_write(proc_t *const p, const sys_args_t *args)
{
	Syscall_reg_write(&ks, p->pid, args->reg_write.reg,
			  args->reg_write.val);
	return p;
}

proc_t *reg_read(proc_t *const p, const sys_args_t *args)
{
	Syscall_reg_read(&ks, p->pid, args->reg_read.reg);
	return p;
}

proc_t *_sync(proc_t *const p, const sys_args_t *args)
{
	(void)args;
	p->timeout = 0;
	return NULL;
}

proc_t *sleep(proc_t *const p, const sys_args_t *args)
{
	Syscall_sleep(&ks, p->pid, args->sleep.time);
	return NULL;
}

proc_t *cap_read(proc_t *const p, const sys_args_t *args)
{
	Syscall_cap_read(&ks, p->pid, args->cap_read.idx);
	return p;
}

proc_t *cap_move(proc_t *const p, const sys_args_t *args)
{
	Syscall_cap_move(&ks, p->pid, args->cap_move.src_idx,
			 args->cap_move.dst_idx);
	return p;
}

proc_t *cap_delete(proc_t *const p, const sys_args_t *args)
{
	Syscall_cap_delete(&ks, p->pid, args->cap_delete.idx);
	return p;
}

proc_t *cap_derive(proc_t *const p, const sys_args_t *args)
{
	Syscall_cap_derive(&ks, p->pid, args->cap_derive.src_idx,
			   args->cap_derive.dst_idx, args->cap_derive.cap_raw);
	return p;
}

proc_t *cap_revoke(proc_t *const p, const sys_args_t *args)
{
	Syscall_cap_revoke(&ks, p->pid, args->cap_revoke.idx);
	return p->t0 == (u64)Error_PREEMPTED ? (proc_t *)NULL : p;
}

proc_t *pmp_load(proc_t *const p, const sys_args_t *args)
{
	Syscall_pmp_load(&ks, p->pid, args->pmp_load.idx, args->pmp_load.slot);
	return p;
}

proc_t *pmp_unload(proc_t *const p, const sys_args_t *args)
{
	Syscall_pmp_unload(&ks, p->pid, args->pmp_unload.idx);
	return p;
}

proc_t *mon_suspend(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_suspend(&ks, p->pid, args->mon_state.mon_idx,
			    args->mon_state.pid);
	return p;
}

proc_t *mon_resume(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_resume(&ks, p->pid, args->mon_state.mon_idx,
			   args->mon_state.pid);
	return p;
}

proc_t *mon_state_get(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_state_get(&ks, p->pid, args->mon_state.mon_idx,
			      args->mon_state.pid);
	return p;
}

proc_t *mon_yield(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_yield(&ks, p->pid, args->mon_state.mon_idx,
			  args->mon_state.pid);
	return next_proc();
}

proc_t *mon_reg_read(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_reg_read(&ks, p->pid, args->mon_reg_read.mon_idx,
			     args->mon_reg_read.pid, args->mon_reg_read.reg);
	return p;
}

proc_t *mon_reg_write(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_reg_write(&ks, p->pid, args->mon_reg_write.mon_idx,
			      args->mon_reg_write.pid, args->mon_reg_write.reg,
			      args->mon_reg_write.val);
	return p;
}

proc_t *mon_cap_read(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_cap_read(&ks, p->pid, args->mon_cap_read.mon_idx,
			     args->mon_cap_read.pid, args->mon_cap_read.idx);
	return p;
}

proc_t *mon_cap_move(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_cap_move(&ks, p->pid, args->mon_cap_move.mon_idx,
			     args->mon_cap_move.src_pid,
			     args->mon_cap_move.src_idx,
			     args->mon_cap_move.dst_pid,
			     args->mon_cap_move.dst_idx);
	return p;
}

proc_t *mon_pmp_load(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_pmp_load(&ks, p->pid, args->mon_pmp_load.mon_idx,
			     args->mon_pmp_load.pid, args->mon_pmp_load.idx,
			     args->mon_pmp_load.slot);
	return p;
}

proc_t *mon_pmp_unload(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_pmp_unload(&ks, p->pid, args->mon_pmp_unload.mon_idx,
			       args->mon_pmp_unload.pid,
			       args->mon_pmp_unload.idx);
	return p;
}

proc_t *sock_send(proc_t *const p, const sys_args_t *args)
{
	Syscall_sock_send(&ks, p->pid, args->sock.sock_idx, args->sock.cap_idx,
			  args->sock.send_cap, (u64 *)args->sock.data);
	return next_proc();
}

proc_t *sock_recv(proc_t *const p, const sys_args_t *args)
{
	Syscall_sock_recv(&ks, p->pid, args->sock.sock_idx, args->sock.cap_idx);
	return next_proc();
}

proc_t *sock_sendrecv(proc_t *const p, const sys_args_t *args)
{
	Syscall_sock_sendrecv(&ks, p->pid, args->sock.sock_idx,
			      args->sock.cap_idx, args->sock.send_cap,
			      (u64 *)args->sock.data);
	return next_proc();
}

proc_t *syscall_handler(proc_t *proc)
{
	// System call arguments.
	const sys_args_t *args = (sys_args_t *)&proc->a0;
	uint64_t call = proc->t0;

	proc->pc += 4;

	switch (call) {
	case SYSCALL_GET_INFO:
		return get_info(proc, args);
	case SYSCALL_REG_READ:
		return reg_read(proc, args);
	case SYSCALL_REG_WRITE:
		return reg_write(proc, args);
	case SYSCALL_SYNC:
		return _sync(proc, args);
	case SYSCALL_SLEEP:
		return sleep(proc, args);
	case SYSCALL_CAP_READ:
		return cap_read(proc, args);
	case SYSCALL_CAP_MOVE:
		return cap_move(proc, args);
	case SYSCALL_CAP_DELETE:
		return cap_delete(proc, args);
	case SYSCALL_CAP_DERIVE:
		return cap_derive(proc, args);
	case SYSCALL_CAP_REVOKE:
		return cap_revoke(proc, args);
	case SYSCALL_PMP_LOAD:
		return pmp_load(proc, args);
	case SYSCALL_PMP_UNLOAD:
		return pmp_unload(proc, args);
	case SYSCALL_MON_SUSPEND:
		return mon_suspend(proc, args);
	case SYSCALL_MON_RESUME:
		return mon_resume(proc, args);
	case SYSCALL_MON_STATE_GET:
		return mon_state_get(proc, args);
	case SYSCALL_MON_YIELD:
		return mon_yield(proc, args);
	case SYSCALL_MON_REG_READ:
		return mon_reg_read(proc, args);
	case SYSCALL_MON_REG_WRITE:
		return mon_reg_write(proc, args);
	case SYSCALL_MON_CAP_READ:
		return mon_cap_read(proc, args);
	case SYSCALL_MON_CAP_MOVE:
		return mon_cap_move(proc, args);
	case SYSCALL_MON_PMP_LOAD:
		return mon_pmp_load(proc, args);
	case SYSCALL_MON_PMP_UNLOAD:
		return mon_pmp_unload(proc, args);
	case SYSCALL_SOCK_SEND:
		return sock_send(proc, args);
	case SYSCALL_SOCK_RECV:
		return sock_recv(proc, args);
	case SYSCALL_SOCK_SENDRECV:
		return sock_sendrecv(proc, args);
	default:
		proc->t0 = Error_INVALID_SYSCALL;
		return proc;
	}
}
