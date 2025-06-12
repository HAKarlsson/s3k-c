#include "cap/util.h"
#include "kstate.h"
#include "libkernel.h"
#include "machine.h"
#include "pmp.h"
#include "proc.h"
#include "sched.h"
#include "types.h"
#include "unity.h"
#include "rtc.h"

extern struct Types_kstate ks;

/*
 * Initializes the kernel state with a set of capabilities.
 * Set the initial process state to ready and the program counter to 0x80010000.
 * Initializes the RTC time and timeout to zero.
 */
void setUp(void)
{
	cap_t init_caps[] = {
	    cap_mk_pmp(pmp_napot_encode(0x80010000, 0x10000), MEM_RWX),
	    cap_mk_memory(0x80020000, 0x80100000, MEM_RWX),
	    cap_mk_memory(0x10000000, 0x10010000, MEM_RW),
	    cap_mk_time(0, S3K_SLOT_CNT),
	    cap_mk_monitor(0, S3K_PROC_CNT),
	};
	kstate_init(init_caps, ARRAY_SIZE(init_caps));
	ks.ptable[0]->pc = 0x80010000;
	ks.ptable[0]->state = 0; // Set state to ready
	Syscall_pmp_load(&ks, 0, 0, 0);
	rtc_time_set(0);
	rtc_timeout_set(0, 0);
}

void tearDown(void)
{
	/* nop */
}

/*
 * Check that process pid is scheduled between time slots bgn and end.
 */
bool check_schedule(pid_t pid, time_slot_t bgn, time_slot_t end)
{
	for (time_slot_t i = bgn; i < end; i++) {
		u64 expected = (pid << 8) | (end - i);
		if (ks.tslots[i] != expected) {
			return false;
		}
	}
	return true;
}

/*
 * Check that no process is not scheduled between time slots bgn and end.
 */
bool check_schedule_deleted(time_slot_t bgn, time_slot_t end)
{
	for (time_slot_t i = bgn; i < end; i++) {
		if (ks.tslots[i] != 0) {
			return false;
		}
	}
	return true;
}

/*
 * Test that barocq memory capability and C memory capability are serialized identically.
 */
void test_cap_memory_serialization(void)
{
	cap_t cap = {.raw = 0x123456789abcdef0ull };
	cap.type = CAPTY_MEMORY;
	TEST_ASSERT_EQUAL_UINT64(cap.type, Cap_get_type(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mem.rwx, Cap_memory_get_rwx(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mem.bgn, Cap_memory_get_bgn(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mem.end, Cap_memory_get_end(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mem.tag, Cap_memory_get_tag(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mem.lck, Cap_memory_get_lck(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mem.mrk, Cap_memory_get_mrk(cap.raw));
}
/*
 * Test that barocq memory capability and C memory capability are serialized identically.
 */
void test_cap_pmp_serialization(void)
{
	cap_t cap = {.raw = 0x123456789abcdef0ull };
	cap.type = CAPTY_PMP;
	TEST_ASSERT_EQUAL_UINT64(cap.type, Cap_get_type(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.pmp.rwx, Cap_pmp_get_rwx(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.pmp.addr, Cap_pmp_get_addr(cap.raw));
}

/*
 * Test that barocq time capability and C time capability are serialized identically.
 */
void test_cap_time_serialization(void)
{
	cap_t cap = {.raw = 0x123456789abcdef0ull };
	cap.type = CAPTY_TIME;
	cap.time._padding = 0; // Ensure padding is zero
	TEST_ASSERT_EQUAL_UINT64(cap.type, Cap_get_type(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.time.bgn, Cap_time_get_bgn(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.time.end, Cap_time_get_end(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.time.mrk, Cap_time_get_mrk(cap.raw));
}

/*
 * Test that barocq monitor capability and C monitor capability are serialized identically.
 */
void test_cap_monitor_serialization(void)
{
	cap_t cap = {.raw = 0x123456789abcdef0ull };
	cap.type = CAPTY_MONITOR;
	cap.mon._padding = 0; // Ensure padding is zero
	TEST_ASSERT_EQUAL_UINT64(cap.type, Cap_get_type(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mon.bgn, Cap_monitor_get_bgn(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mon.end, Cap_monitor_get_end(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mon.mrk, Cap_monitor_get_mrk(cap.raw));
}

/*
 * Test that the kernel is setup correctly.
 */
void test_Setup(void)
{
	uint64_t ram_pmp = pmp_napot_encode(0x80010000, 0x10000);
	u64 init_caps[] = {
	    cap_mk_pmp(ram_pmp, MEM_RWX).raw,
	    cap_mk_memory(0x80020000, 0x80100000, MEM_RWX).raw,
	    cap_mk_memory(0x10000000, 0x10010000, MEM_RW).raw,
	    cap_mk_time(0, S3K_SLOT_CNT).raw,
	    cap_mk_monitor(0, S3K_PROC_CNT).raw,
	};
	init_caps[0] = Cap_pmp_set_slot(init_caps[0], 0); // Set slot to 0
	init_caps[0] = Cap_pmp_set_used(init_caps[0], true); // Set slot to 0

	TEST_ASSERT_EQUAL_UINT64_ARRAY(init_caps, ks.ctable, ARRAY_SIZE(init_caps));

	// Check PC initialized to 0x80010000
	TEST_ASSERT_EQUAL_UINT64(ks.ptable[0]->pc, 0x80010000);
	// Check state initialized to ready (= 0)
	TEST_ASSERT_EQUAL_UINT64(ks.ptable[0]->state, 0);
	// Check PMP initialized correctly
	TEST_ASSERT_EQUAL_UINT64(ks.ptable[0]->pmpcfg[0], MEM_RWX | 0x18);
	TEST_ASSERT_EQUAL_UINT64(ks.ptable[0]->pmpaddr[0], ram_pmp);
	for (int i = 1; i < S3K_PMP_CNT; i++) {
		TEST_ASSERT_EQUAL_UINT64(ks.ptable[0]->pmpcfg[i], 0);
		TEST_ASSERT_EQUAL_UINT64(ks.ptable[0]->pmpaddr[i], 0);
	}

	// Check other processes
	for (int i = 1; i < S3K_PROC_CNT; i++) {
		TEST_ASSERT_EQUAL_UINT64(ks.ptable[i]->state, Proc_PSF_SUSPENDED);
		TEST_ASSERT_EQUAL_UINT64(ks.ptable[i]->pid, i);
		TEST_ASSERT_EQUAL_UINT64(ks.ptable[i]->pc, 0);
		for (int j = 0; j < S3K_PMP_CNT; j++) {
			TEST_ASSERT_EQUAL_UINT64(ks.ptable[i]->pmpcfg[j], 0);
			TEST_ASSERT_EQUAL_UINT64(ks.ptable[i]->pmpaddr[j], 0);
		}
	}

	TEST_ASSERT_TRUE(check_schedule(0, 0, S3K_SLOT_CNT));
}

void test_Syscall_cap_read(void)
{
	// Process 0 reads capability 0.
	Syscall_cap_read(&ks, 0, 0);
	TEST_ASSERT_EQUAL_UINT64(ks.ptable[0]->t0, Error_SUCCESS);
	TEST_ASSERT_EQUAL_UINT64(ks.ptable[0]->a0, ks.ctable[0]);

	// Process 0 reads capability 1.
	Syscall_cap_read(&ks, 0, 1);
	TEST_ASSERT_EQUAL_UINT64(ks.ptable[0]->t0, Error_SUCCESS);
	TEST_ASSERT_EQUAL_UINT64(ks.ptable[0]->a0, ks.ctable[1]);
}

void test_Syscall_cap_move(void)
{
	// Process 0 moves capability 0 to slot 10.
	u64 cap0 = ks.ctable[0];
	Syscall_cap_move(&ks, 0, 0, 8);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[0]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap0, ks.ctable[8]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[0]);

	// Process 0 moves capability 8 to slot 100. (Should fail)
	Syscall_cap_move(&ks, 0, 8, 100);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_INDEX, ks.ptable[0]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap0, ks.ctable[8]);

	// Process 0 moves capability 1 to slot 0. (Should fail)
	u64 cap1 = ks.ctable[1];
	Syscall_cap_move(&ks, 0, 1, 0);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[0]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap1, ks.ctable[0]);
}

void test_Syscall_cap_delete(void)
{
	// Process 0 deletes capability 0.
	Syscall_cap_delete(&ks, 0, 0);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[0]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[0]);

	// Process 0 deletes capability 1.
	Syscall_cap_delete(&ks, 0, 1);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[0]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[1]);

	// Process 0 tries to delete capability 1 again. (Should fail)
	Syscall_cap_delete(&ks, 0, 1);
	TEST_ASSERT_EQUAL_UINT64(Error_EMPTY, ks.ptable[0]->t0);
}

void test_Syscall_cap_derive_memory_valid1(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 8; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80030000, MEM_RWX);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst]);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap.raw, ks.ctable[dst]);
}

void test_Syscall_cap_derive_memory_valid2(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst1 = 6; // Destination capability index
	int dst2 = 7; // Another destination capability index
	cap_t cap1 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	cap_t cap2 = cap_mk_memory(0x80040000, 0x80080000, MEM_RW);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, src, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_valid3(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst1 = 6; // Destination capability index
	int dst2 = 7; // Another destination capability index
	cap_t cap1 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	cap_t cap2 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, dst1, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_invalid_double1(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst1 = 6; // Destination capability index
	int dst2 = 7; // Another destination capability index
	cap_t cap1 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	cap_t cap2 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, src, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_invalid_tripple(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst1 = 6; // Destination capability index
	int dst2 = 7; // Another destination capability index
	int dst3 = 8; // Another destination capability index
	cap_t cap1 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	cap_t cap2 = cap_mk_pmp(pmp_napot_encode(0x80040000, 0x10000), MEM_RWX);
	cap_t cap3 = cap_mk_memory(0x80040000, 0x80080000, MEM_RW);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, src, dst2, cap2.raw);
	Syscall_cap_derive(&ks, pid, src, dst3, cap3.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_invalid_dest1(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = Config_S3K_CAP_CNT; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80030000, MEM_RWX);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst]);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_INDEX, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_invalid_dest2(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 2; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80030000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_DST_OCCUPIED, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_invalid_dest3(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 1; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80030000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_DST_OCCUPIED, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_invalid_range1(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 6; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80020000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_invalid_range2(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 6; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80010000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_invalid_range3(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 6; // Destination capability index
	cap_t cap = cap_mk_memory(0x80010000, 0x80040000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_invalid_range4(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 6; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80140000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_invalid_perm1(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst1 = 6; // Destination capability index
	int dst2 = 7; // Another destination capability index
	cap_t cap1 = cap_mk_memory(0x80020000, 0x80040000, MEM_R);
	cap_t cap2 = cap_mk_memory(0x80020000, 0x80040000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, dst1, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_memory_invalid_perm2(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst1 = 6; // Destination capability index
	int dst2 = 7; // Another destination capability index
	cap_t cap1 = cap_mk_memory(0x80020000, 0x80040000, MEM_R);
	cap_t cap2 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, dst1, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

void test_Syscall_cap_delete_pmp(void)
{
	int pid = 0; // Process ID
	int src = 0; // Source capability index
	TEST_ASSERT_EQUAL_UINT64(Cap_pmp_get_addr(ks.ctable[src]), ks.ptable[pid]->pmpaddr[0]);
	Syscall_cap_delete(&ks, pid, src);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[src]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ptable[pid]->pmpcfg[0]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ptable[pid]->pmpaddr[0]);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
}

void test_Syscall_cap_derive_time_valid1(void)
{
	int pid = 0; // Process ID
	int src = 3; // Source capability index
	int dst = 8; // Destination capability index
	cap_t cap = cap_mk_time(0, 16);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst]);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap.raw, ks.ctable[dst]);
	TEST_ASSERT_TRUE(check_schedule(pid, 0, 16));
	TEST_ASSERT_TRUE(check_schedule(pid, 16, 32));
}

/**
 * Delete the root capability
 */
void test_Syscall_cap_delete_time1(void)
{
	int pid = 0; // Process ID
	int src = 3; // Source capability index
	Syscall_cap_delete(&ks, pid, src);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[src]);
	TEST_ASSERT_TRUE(check_schedule_deleted(0, S3K_SLOT_CNT));
}

/**
 * Derive then delete the root capability
 */
void test_Syscall_cap_derive_delete_time1(void)
{
	int pid = 0; // Process ID
	int src = 3; // Source capability index
	int dst = 8; // Destination capability index
	cap_t cap = cap_mk_time(0, 16);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst]);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	Syscall_cap_delete(&ks, pid, src);
	TEST_ASSERT_TRUE(check_schedule(pid, 0, 16));
	TEST_ASSERT_TRUE(check_schedule_deleted(16, 32));
}

/**
 * Derive then delete the derived capability
 */
void test_Syscall_cap_derive_delete_time2(void)
{
	int pid = 0; // Process ID
	int src = 3; // Source capability index
	int dst = 8; // Destination capability index
	cap_t cap = cap_mk_time(0, 16);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst]);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	Syscall_cap_delete(&ks, pid, dst);
	TEST_ASSERT_TRUE(check_schedule_deleted(0, 16));
	TEST_ASSERT_TRUE(check_schedule(pid, 16, 32));
}

void test_Syscall_cap_revoke_time1(void)
{
	rtc_time_set(0);
	rtc_timeout_set(0, 1000); // Reset timeout
	int pid = 0; // Process ID
	int src = 3; // Source capability index
	int dst = 8; // Destination capability index
	cap_t cap = cap_mk_time(0, 16);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst]);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	Syscall_cap_revoke(&ks, pid, dst);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_TRUE(check_schedule(pid, 0, 16));
	TEST_ASSERT_TRUE(check_schedule(pid, 16, S3K_SLOT_CNT));
}
