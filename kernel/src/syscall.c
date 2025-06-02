/* See LICENSE file for copyright and license details. */
#include "syscall.h"

#include "cap/ipc.h"
#include "cap/monitor.h"
#include "cap/ops.h"
#include "cap/pmp.h"
#include "cap/table.h"
#include "cap/util.h"
#include "csr.h"
#include "kprint.h"
#include "libkernel.h"
#include "rtc.h"
#include "sched.h"
#include "trap.h"
#include "types.h"

#include <stdbool.h>

extern struct Kernel_state ks;

static bool _valid_idx(cidx_t idx)
{
	return idx < S3K_CAP_CNT;
}

static bool _valid_slot(pmp_slot_t slot)
{
	return slot < S3K_PMP_CNT;
}

static bool _valid_pid(pid_t pid)
{
	return pid < S3K_PROC_CNT;
}

static bool _valid_reg(word_t reg)
{
	size_t reg_count = 32;
	return reg < reg_count;
}

proc_t *_get_info(proc_t *const p, const sys_args_t *args)
{
	switch (args->get_info.info) {
	case 0:
		p->a0 = p->pid;
		break;
	case 1:
		p->a0 = rtc_time_get();
		break;
	case 2:
		p->a0 = rtc_timeout_get(csrr_mhartid());
		break;
	default:
		p->a0 = 0;
	}
	p->t0 = SUCCESS;
	return p;
}

proc_t *_reg_read(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_reg(args->reg_read.reg)) {
		p->t0 = ERR_INVALID_REGISTER;
		return p;
	}
	p->t0 = SUCCESS;
	word_t *regs = (word_t *)&p->pc;
	p->a0 = regs[args->reg_read.reg];
	return p;
}

proc_t *_reg_write(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_reg(args->reg_write.reg)) {
		p->t0 = ERR_INVALID_REGISTER;
		return p;
	}
	p->t0 = SUCCESS;
	word_t *regs = (word_t *)&p->pc;
	regs[args->reg_write.reg] = args->reg_write.val;
	return p;
}

proc_t *_sync(proc_t *const p, const sys_args_t *args)
{
	(void)args;
	p->timeout = 0;
	return NULL;
}

proc_t *_sleep(proc_t *const p, const sys_args_t *args)
{
	if (args->sleep.time)
		p->timeout = args->sleep.time;
	return NULL;
}

proc_t *_cap_read(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->cap_read.idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t c = ctable_get(p->pid, args->cap_read.idx);
	p->t0 = cap_read(c, (cap_t *)&p->a0);
	return p;
}

proc_t *_cap_move(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->cap_move.src_idx)
	    || !_valid_idx(args->cap_move.dst_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t src = ctable_get(p->pid, args->cap_move.src_idx);
	cte_t dst = ctable_get(p->pid, args->cap_move.dst_idx);
	p->t0 = cap_move(src, dst);
	return p;
}

proc_t *_cap_delete(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->cap_delete.idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t c = ctable_get(p->pid, args->cap_delete.idx);
	p->t0 = cap_delete(c);
	return p;
}

proc_t *_cap_revoke(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->cap_revoke.idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t c = ctable_get(p->pid, args->cap_revoke.idx);

	p->t0 = cap_revoke(c);
	return p->t0 == ERR_PREEMPTED ? (proc_t *)NULL : p;
}

proc_t *_cap_derive(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->cap_derive.src_idx)
	    || !_valid_idx(args->cap_derive.dst_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cap_t cap = {.raw = args->cap_derive.cap_raw};
	if (!cap_is_valid(&cap)) {
		p->t0 = ERR_INVALID_DERIVATION;
		return p;
	}
	cte_t src = ctable_get(p->pid, args->cap_derive.src_idx);
	cte_t dst = ctable_get(p->pid, args->cap_derive.dst_idx);
	p->t0 = cap_derive(src, dst, &cap);
	return p;
}

proc_t *_pmp_load(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->pmp_load.idx)
	    || !_valid_slot(args->pmp_load.slot)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t pmp = ctable_get(p->pid, args->pmp_load.idx);
	p->t0 = cap_pmp_load(pmp, args->pmp_load.slot);
	return p;
}

proc_t *_pmp_unload(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->pmp_unload.idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t pmp = ctable_get(p->pid, args->pmp_unload.idx);
	p->t0 = cap_pmp_unload(pmp);
	return p;
}

proc_t *_mon_suspend(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->mon_state.mon_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_pid(args->mon_state.pid)) {
		p->t0 = ERR_INVALID_PID;
		return p;
	}
	cte_t mon = ctable_get(p->pid, args->mon_state.mon_idx);
	p->t0 = cap_monitor_suspend(mon, args->mon_state.pid);
	return p;
}

proc_t *_mon_resume(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->mon_state.mon_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_pid(args->mon_state.pid)) {
		p->t0 = ERR_INVALID_PID;
		return p;
	}
	cte_t mon = ctable_get(p->pid, args->mon_state.mon_idx);
	p->t0 = cap_monitor_resume(mon, args->mon_state.pid);
	return p;
}

proc_t *_mon_state_get(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->mon_state.mon_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_pid(args->mon_state.pid)) {
		p->t0 = ERR_INVALID_PID;
		return p;
	}
	cte_t mon = ctable_get(p->pid, args->mon_state.mon_idx);
	p->t0 = cap_monitor_state_get(mon, args->mon_state.pid,
				      (proc_state_t *)&p->a0);
	return p;
}

proc_t *_mon_yield(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->mon_state.mon_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_pid(args->mon_state.pid)) {
		p->t0 = ERR_INVALID_PID;
		return p;
	}
	cte_t mon = ctable_get(p->pid, args->mon_state.mon_idx);
	proc_t *next = p;
	p->t0 = cap_monitor_yield(mon, args->mon_state.pid, &next);
	return next;
}

proc_t *_mon_reg_read(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->mon_reg_read.mon_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_pid(args->mon_reg_read.pid)) {
		p->t0 = ERR_INVALID_PID;
		return p;
	}
	if (!_valid_reg(args->mon_reg_read.reg)) {
		p->t0 = ERR_INVALID_REGISTER;
		return p;
	}
	cte_t mon = ctable_get(p->pid, args->mon_reg_read.mon_idx);
	word_t *a0 = (word_t *)(&p->a0);
	p->t0 = cap_monitor_reg_read(mon, args->mon_reg_read.pid,
				     args->mon_reg_read.reg, a0);
	return p;
}

proc_t *_mon_reg_write(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->mon_reg_write.mon_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_pid(args->mon_reg_write.pid)) {
		p->t0 = ERR_INVALID_PID;
		return p;
	}
	if (!_valid_reg(args->mon_reg_write.reg)) {
		p->t0 = ERR_INVALID_REGISTER;
		return p;
	}
	cte_t mon = ctable_get(p->pid, args->mon_reg_write.mon_idx);
	p->t0 = cap_monitor_reg_write(mon, args->mon_reg_write.pid,
				      args->mon_reg_write.reg,
				      args->mon_reg_write.val);
	return p;
}

proc_t *_mon_cap_read(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->mon_cap_read.mon_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_pid(args->mon_cap_read.pid)) {
		p->t0 = ERR_INVALID_PID;
		return p;
	}
	if (!_valid_idx(args->mon_cap_read.idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t mon = ctable_get(p->pid, args->mon_cap_read.mon_idx);
	cte_t src = ctable_get(args->mon_cap_read.pid, args->mon_cap_read.idx);
	p->t0 = cap_monitor_cap_read(mon, src, (cap_t *)&p->a0);
	return p;
}

proc_t *_mon_cap_move(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->mon_cap_move.mon_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_pid(args->mon_cap_move.src_pid)) {
		p->t0 = ERR_INVALID_PID;
		return p;
	}
	if (!_valid_idx(args->mon_cap_move.src_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_pid(args->mon_cap_move.dst_pid)) {
		p->t0 = ERR_INVALID_PID;
		return p;
	}
	if (!_valid_idx(args->mon_cap_move.dst_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t mon = ctable_get(p->pid, args->mon_cap_move.mon_idx);
	cte_t src = ctable_get(args->mon_cap_move.src_pid,
			       args->mon_cap_move.src_idx);
	cte_t dst = ctable_get(args->mon_cap_move.dst_pid,
			       args->mon_cap_move.dst_idx);
	p->t0 = cap_monitor_cap_move(mon, src, dst);
	return p;
}

proc_t *_mon_pmp_load(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->mon_pmp_load.mon_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_pid(args->mon_pmp_load.pid)) {
		p->t0 = ERR_INVALID_PID;
		return p;
	}
	if (!_valid_idx(args->mon_pmp_load.idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_slot(args->mon_pmp_load.slot)) {
		p->t0 = ERR_INVALID_SLOT;
		return p;
	}
	cte_t mon = ctable_get(p->pid, args->mon_pmp_load.mon_idx);
	cte_t pmp = ctable_get(args->mon_pmp_load.pid, args->mon_pmp_load.idx);
	p->t0 = cap_monitor_pmp_load(mon, pmp, args->mon_pmp_load.slot);
	return p;
}

proc_t *_mon_pmp_unload(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->mon_pmp_unload.mon_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_pid(args->mon_pmp_unload.pid)) {
		p->t0 = ERR_INVALID_PID;
		return p;
	}
	if (!_valid_idx(args->mon_pmp_unload.idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t mon = ctable_get(p->pid, args->mon_pmp_unload.mon_idx);
	cte_t pmp
	    = ctable_get(args->mon_pmp_unload.pid, args->mon_pmp_unload.idx);
	p->t0 = cap_monitor_pmp_unload(mon, pmp);
	return p;
}

proc_t *_sock_send(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->sock.sock_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_idx(args->sock.cap_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t sock = ctable_get(p->pid, args->sock.sock_idx);
	const ipc_msg_t msg = {
	    .cap_buf = ctable_get(p->pid, args->sock.cap_idx),
	    .send_cap = args->sock.send_cap,
	    .data = {args->sock.data[0], args->sock.data[1], args->sock.data[2],
		     args->sock.data[3]},
	};
	proc_t *next = p;
	p->t0 = cap_sock_send(sock, &msg, &next);
	return next;
}

proc_t *_sock_recv(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->sock.sock_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_idx(args->sock.cap_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t sock = ctable_get(p->pid, args->sock.sock_idx);
	cte_t cap_buf = ctable_get(p->pid, args->sock.cap_idx);
	proc_t *next = p;
	p->t0 = cap_sock_recv(sock, cap_buf, &next);
	return next;
}

proc_t *_sock_sendrecv(proc_t *const p, const sys_args_t *args)
{
	if (!_valid_idx(args->sock.sock_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	if (!_valid_idx(args->sock.cap_idx)) {
		p->t0 = ERR_INVALID_INDEX;
		return p;
	}
	cte_t sock = ctable_get(p->pid, args->sock.sock_idx);
	const ipc_msg_t msg = {
	    .cap_buf = ctable_get(p->pid, args->sock.cap_idx),
	    .send_cap = args->sock.send_cap,
	    .data = {args->sock.data[0], args->sock.data[1], args->sock.data[2],
		     args->sock.data[3]},
	};
	proc_t *next = p;
	p->t0 = cap_sock_sendrecv(sock, &msg, &next);
	return next;
}

proc_t *_br_cap_read(proc_t *const p, const sys_args_t *args)
{
	Syscall_cap_read(&ks, p->pid, args->cap_read.idx);
	return p;
}

proc_t *_br_cap_move(proc_t *const p, const sys_args_t *args)
{
	Syscall_cap_move(&ks, p->pid, args->cap_move.src_idx,
			 args->cap_move.dst_idx);
	return p;
}

proc_t *_br_cap_delete(proc_t *const p, const sys_args_t *args)
{
	Syscall_cap_delete(&ks, p->pid, args->cap_delete.idx);
	return p;
}

proc_t *_br_cap_derive(proc_t *const p, const sys_args_t *args)
{
	Syscall_cap_derive(&ks, p->pid, args->cap_derive.src_idx,
			   args->cap_derive.dst_idx, args->cap_derive.cap_raw);
	return p;
}

proc_t *_br_cap_revoke(proc_t *const p, const sys_args_t *args)
{
	return p->t0 == ERR_PREEMPTED ? (proc_t *)NULL : p;
}

proc_t *_br_pmp_load(proc_t *const p, const sys_args_t *args)
{
	Syscall_pmp_load(&ks, p->pid, args->pmp_load.idx, args->pmp_load.slot);
	return p;
}

proc_t *_br_pmp_unload(proc_t *const p, const sys_args_t *args)
{
	Syscall_pmp_unload(&ks, p->pid, args->pmp_unload.idx);
	return p;
}

proc_t *_br_mon_suspend(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_suspend(&ks, p->pid, args->mon_state.mon_idx,
			    args->mon_state.pid);
	return p;
}

proc_t *_br_mon_resume(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_resume(&ks, p->pid, args->mon_state.mon_idx,
			   args->mon_state.pid);
	return p;
}

proc_t *_br_mon_state_get(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_state_get(&ks, p->pid, args->mon_state.mon_idx,
			      args->mon_state.pid);
	return p;
}

proc_t *_br_mon_yield(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_yield(&ks, p->pid, args->mon_state.mon_idx,
			  args->mon_state.pid);
	proc_t *next = ks.ptable[Kernel_vreg_read(&ks, 0)];
	return next;
}

proc_t *_br_mon_reg_read(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_reg_read(&ks, p->pid, args->mon_reg_read.mon_idx,
			     args->mon_reg_read.pid, args->mon_reg_read.reg);
	return p;
}

proc_t *_br_mon_reg_write(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_reg_write(&ks, p->pid, args->mon_reg_write.mon_idx,
			      args->mon_reg_write.pid, args->mon_reg_write.reg,
			      args->mon_reg_write.val);
	return p;
}

proc_t *_br_mon_cap_read(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_cap_read(&ks, p->pid, args->mon_cap_read.mon_idx,
			     args->mon_cap_read.pid, args->mon_cap_read.idx);
	return p;
}

proc_t *_br_mon_cap_move(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_cap_move(&ks, p->pid, args->mon_cap_move.mon_idx,
			     args->mon_cap_move.src_pid,
			     args->mon_cap_move.src_idx,
			     args->mon_cap_move.dst_pid,
			     args->mon_cap_move.dst_idx);
	return p;
}

proc_t *_br_mon_pmp_load(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_pmp_load(&ks, p->pid, args->mon_pmp_load.mon_idx,
			     args->mon_pmp_load.pid, args->mon_pmp_load.idx,
			     args->mon_pmp_load.slot);
	return p;
}

proc_t *_br_mon_pmp_unload(proc_t *const p, const sys_args_t *args)
{
	Syscall_mon_pmp_unload(&ks, p->pid, args->mon_pmp_unload.mon_idx,
			       args->mon_pmp_unload.pid,
			       args->mon_pmp_unload.idx);
	return p;
}

proc_t *syscall_handler(proc_t *proc)
{
	// System call arguments.
	const sys_args_t *args = (sys_args_t *)&proc->a0;
	uint64_t call = proc->t0;

	proc->pc += 4;

	switch (call) {
	case SYSCALL_GET_INFO:
		return _get_info(proc, args);
	case SYSCALL_REG_READ:
		return _reg_read(proc, args);
	case SYSCALL_REG_WRITE:
		return _reg_write(proc, args);
	case SYSCALL_SYNC:
		return _sync(proc, args);
	case SYSCALL_SLEEP:
		return _sleep(proc, args);
	case SYSCALL_CAP_READ:
		return _cap_read(proc, args);
	case SYSCALL_CAP_MOVE:
		return _cap_move(proc, args);
	case SYSCALL_CAP_DELETE:
		return _cap_delete(proc, args);
	case SYSCALL_CAP_REVOKE:
		return _cap_revoke(proc, args);
	case SYSCALL_CAP_DERIVE:
		return _cap_derive(proc, args);
	case SYSCALL_PMP_LOAD:
		return _pmp_load(proc, args);
	case SYSCALL_PMP_UNLOAD:
		return _pmp_unload(proc, args);
	case SYSCALL_MON_SUSPEND:
		return _mon_suspend(proc, args);
	case SYSCALL_MON_RESUME:
		return _mon_resume(proc, args);
	case SYSCALL_MON_STATE_GET:
		return _mon_state_get(proc, args);
	case SYSCALL_MON_YIELD:
		return _mon_yield(proc, args);
	case SYSCALL_MON_REG_READ:
		return _mon_reg_read(proc, args);
	case SYSCALL_MON_REG_WRITE:
		return _mon_reg_write(proc, args);
	case SYSCALL_MON_CAP_READ:
		return _mon_cap_read(proc, args);
	case SYSCALL_MON_CAP_MOVE:
		return _mon_cap_move(proc, args);
	case SYSCALL_MON_PMP_LOAD:
		return _mon_pmp_load(proc, args);
	case SYSCALL_MON_PMP_UNLOAD:
		return _mon_pmp_unload(proc, args);
	case SYSCALL_SOCK_SEND:
		return _sock_send(proc, args);
	case SYSCALL_SOCK_RECV:
		return _sock_recv(proc, args);
	case SYSCALL_SOCK_SENDRECV:
		return _sock_sendrecv(proc, args);
	case BR_SYSCALL_CAP_READ:
		return _br_cap_read(proc, args);
	case BR_SYSCALL_CAP_MOVE:
		return _br_cap_move(proc, args);
	case BR_SYSCALL_CAP_DELETE:
		return _br_cap_delete(proc, args);
	case BR_SYSCALL_CAP_DERIVE:
		return _br_cap_derive(proc, args);
	case BR_SYSCALL_CAP_REVOKE:
		return _br_cap_revoke(proc, args);
	case BR_SYSCALL_PMP_LOAD:
		return _br_pmp_load(proc, args);
	case BR_SYSCALL_PMP_UNLOAD:
		return _br_pmp_unload(proc, args);
	case BR_SYSCALL_MON_SUSPEND:
		return _br_mon_suspend(proc, args);
	case BR_SYSCALL_MON_RESUME:
		return _br_mon_resume(proc, args);
	case BR_SYSCALL_MON_STATE_GET:
		return _br_mon_state_get(proc, args);
	case BR_SYSCALL_MON_YIELD:
		return _br_mon_yield(proc, args);
	case BR_SYSCALL_MON_REG_READ:
		return _br_mon_reg_read(proc, args);
	case BR_SYSCALL_MON_REG_WRITE:
		return _br_mon_reg_write(proc, args);
	case BR_SYSCALL_MON_CAP_READ:
		return _br_mon_cap_read(proc, args);
	case BR_SYSCALL_MON_CAP_MOVE:
		return _br_mon_cap_move(proc, args);
	case BR_SYSCALL_MON_PMP_LOAD:
		return _br_mon_pmp_load(proc, args);
	case BR_SYSCALL_MON_PMP_UNLOAD:
		return _br_mon_pmp_unload(proc, args);
	default:
		proc->t0 = ERR_INVALID_SYSCALL;
		return proc;
	}
}