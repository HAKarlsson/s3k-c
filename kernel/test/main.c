#include "cap/util.h"
#include "kstate.h"
#include "libkernel.h"
#include "machine.h"
#include "pmp.h"
#include "proc.h"
#include "rtc.h"
#include "sched.h"
#include "types.h"
#include "unity.h"

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
	    cap_mk_time(0, Config_S3K_SLOT_CNT),
	    cap_mk_monitor(0, Config_S3K_PROC_CNT),
	    cap_mk_channel(0, Config_S3K_CHAN_CNT),
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
	cap_t cap = {.raw = 0x123456789abcdef0ull};
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
	cap_t cap = {.raw = 0x123456789abcdef0ull};
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
	cap_t cap = {.raw = 0x123456789abcdef0ull};
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
	cap_t cap = {.raw = 0x123456789abcdef0ull};
	cap.type = CAPTY_MONITOR;
	cap.mon._padding = 0; // Ensure padding is zero
	TEST_ASSERT_EQUAL_UINT64(cap.type, Cap_get_type(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mon.bgn, Cap_monitor_get_bgn(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mon.end, Cap_monitor_get_end(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.mon.mrk, Cap_monitor_get_mrk(cap.raw));
}

/*
 * Test that barocq channel capability and C channel capability are serialized identically.
 */
void test_cap_channel_serialization(void)
{
	cap_t cap = {.raw = 0x123456789abcdef0ull};
	cap.type = CAPTY_CHANNEL;
	cap.chan._padding = 0; // Ensure padding is zero
	TEST_ASSERT_EQUAL_UINT64(cap.type, Cap_get_type(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.chan.bgn, Cap_channel_get_bgn(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.chan.end, Cap_channel_get_end(cap.raw));
	TEST_ASSERT_EQUAL_UINT64(cap.chan.mrk, Cap_channel_get_mrk(cap.raw));
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
	    cap_mk_time(0, Config_S3K_SLOT_CNT).raw,
	    cap_mk_monitor(0, Config_S3K_PROC_CNT).raw,
	    cap_mk_channel(0, Config_S3K_CHAN_CNT).raw,
	};
	init_caps[0] = Cap_pmp_set_slot(init_caps[0], 0);    // Set slot to 0
	init_caps[0] = Cap_pmp_set_used(init_caps[0], true); // Set slot to 0

	TEST_ASSERT_EQUAL_UINT64_ARRAY(init_caps, ks.ctable,
				       ARRAY_SIZE(init_caps));

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
		TEST_ASSERT_EQUAL_UINT64(ks.ptable[i]->state,
					 Proc_PSF_SUSPENDED);
		TEST_ASSERT_EQUAL_UINT64(ks.ptable[i]->pid, i);
		TEST_ASSERT_EQUAL_UINT64(ks.ptable[i]->pc, 0);
		for (int j = 0; j < S3K_PMP_CNT; j++) {
			TEST_ASSERT_EQUAL_UINT64(ks.ptable[i]->pmpcfg[j], 0);
			TEST_ASSERT_EQUAL_UINT64(ks.ptable[i]->pmpaddr[j], 0);
		}
	}

	TEST_ASSERT_TRUE(check_schedule(0, 0, S3K_SLOT_CNT));
}

/*
 * Check that Syscall_cap_read reads the correct capability
 */
void test_Syscall_cap_read(void)
{
	for (int i = 0; i < S3K_CAP_CNT; i++) {
		u64 cap = ks.ctable[i];
		u64 a0 = ks.ptable[0]->a0;
		Syscall_cap_read(&ks, 0, i);
		if (cap == 0) {
			TEST_ASSERT_EQUAL_UINT64(Error_EMPTY, ks.ptable[0]->t0);
			TEST_ASSERT_EQUAL_UINT64(a0, ks.ptable[0]->a0);
		} else {
			TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS,
						 ks.ptable[0]->t0);
			TEST_ASSERT_EQUAL_UINT64(cap, ks.ptable[0]->a0);
		}
	}
}

/*
 * Check that Syscall_cap_move moves the capability to the correct slot
 */
void test_Syscall_cap_move_valid1(void)
{
	// Process 0 moves capability 0 to slot 10.
	u64 cap0 = ks.ctable[0];
	Syscall_cap_move(&ks, 0, 0, 8);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[0]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap0, ks.ctable[8]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[0]);
}

/*
 * Check that Syscall_cap_move does not move a capability to an invalid slot
 */
void test_Syscall_cap_move_invalid1(void)
{
	u64 cap0 = ks.ctable[0];
	// Process 0 tries to move capability 0 to slot 100. (Should fail)
	Syscall_cap_move(&ks, 0, 1, 100);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_INDEX, ks.ptable[0]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap0, ks.ctable[0]);
}

/*
 * Check that Syscall_cap_move moves the capability colliding with an existing one
 */
void test_Syscall_cap_move_valid(void)
{
	// Process 0 moves capability 0 to slot 1. (Should fail)
	u64 cap0 = ks.ctable[0];
	u64 cap1 = ks.ctable[1];
	Syscall_cap_move(&ks, 0, 1, 0);
	TEST_ASSERT_EQUAL_UINT64(Error_DST_OCCUPIED, ks.ptable[0]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap0, ks.ctable[0]);
	TEST_ASSERT_EQUAL_UINT64(cap1, ks.ctable[1]);
}

/*
 * Check that Syscall_cap_delete can delete capabilities
 */
void test_Syscall_cap_delete_valid1(void)
{
	// Process 0 deletes capability 0.
	Syscall_cap_delete(&ks, 0, 0);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[0]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[0]);

	// Process 0 deletes capability 1.
	Syscall_cap_delete(&ks, 0, 1);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[0]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[1]);
}

/*
 * Check that Syscall_cap_delete does not delete capabilities that are not present
 */
void test_Syscall_cap_delete_invalid1(void)
{
	// Process 0 tries to delete capability 0 twice.
	Syscall_cap_delete(&ks, 0, 0);
	Syscall_cap_delete(&ks, 0, 0);
	TEST_ASSERT_EQUAL_UINT64(Error_EMPTY, ks.ptable[0]->t0);
	// Process 0 tries to delete capability 10.
	ks.ptable[0]->t0 = 0xdeadbeef;
	Syscall_cap_delete(&ks, 0, 10);
	TEST_ASSERT_EQUAL_UINT64(Error_EMPTY, ks.ptable[0]->t0);
}

/*
 * Check that Syscall_cap_delete does not delete capabilities out of bounds.
 */
void test_Syscall_cap_delete_invalid2(void)
{
	// Process 0 tries to delete capability 32 twice.
	Syscall_cap_delete(&ks, 0, 32);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_INDEX, ks.ptable[0]->t0);
}

/*
 * Check that Syscall_cap_revoke on invalid idnex returns Error_INVALID_INDEX.
 */
void test_Syscall_cap_revoke_invalid_index1(void)
{
	// Process 0 revokes capability 32 (out-of-bounds).
	Syscall_cap_revoke(&ks, 0, 32);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_INDEX, ks.ptable[0]->t0);
}

/*
 * Check that Syscall_cap_revoke on empty capability returns Error_EMPTY.
 */
void test_Syscall_cap_revoke_empty1(void)
{
	// Process 0 revokes capability 8 (empty).
	Syscall_cap_revoke(&ks, 0, 8);
	TEST_ASSERT_EQUAL_UINT64(Error_EMPTY, ks.ptable[0]->t0);
}

/*
 * Check that Syscall_cap_derive can derive a valid memory capability.
 */
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

/*
 * Check that Syscall_cap_derive can derive a valid memory capability.
 */
void test_Syscall_cap_derive_memory_valid2(void)
{
	int pid = 0; // Process ID
	int src = 2; // Source capability index
	int dst = 8; // Destination capability index
	cap_t cap = cap_mk_memory(0x10000000, 0x10001000, MEM_RW);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst]);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap.raw, ks.ctable[dst]);
}

/*
 * Check that non-overlapping memory capabilities can be derived.
 */
void test_Syscall_cap_derive_memory_valid3(void)
{
	int pid = 0;  // Process ID
	int src = 1;  // Source capability index
	int dst1 = 6; // Destination capability index
	int dst2 = 7; // Another destination capability index
	cap_t cap1 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	cap_t cap2 = cap_mk_memory(0x80040000, 0x80080000, MEM_RW);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, src, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
}

/*
 * Check that memory capabilities can be derived recursively.
 */
void test_Syscall_cap_derive_memory_valid4(void)
{
	int pid = 0;  // Process ID
	int src = 1;  // Source capability index
	int dst1 = 6; // Destination capability index
	int dst2 = 7; // Another destination capability index
	cap_t cap1 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	cap_t cap2 = cap_mk_memory(0x80020000, 0x80040000, MEM_R);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, dst1, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
}

/*
 * Check that overlapping memory capabilities cannot be derived from the same parent.
 */
void test_Syscall_cap_derive_memory_invalid_double1(void)
{
	int pid = 0;  // Process ID
	int src = 1;  // Source capability index
	int dst1 = 6; // Destination capability index
	int dst2 = 7; // Another destination capability index
	cap_t cap1 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	cap_t cap2 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, src, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

/*
 * Check that memory capability cannot be derived after a PMP capability.
 */
void test_Syscall_cap_derive_memory_invalid_tripple(void)
{
	int pid = 0;  // Process ID
	int src = 1;  // Source capability index
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

/*
 * Check that a memory capability can not be derived to a invalid destination index.
 */
void test_Syscall_cap_derive_memory_invalid_dest1(void)
{
	int pid = 0;		      // Process ID
	int src = 1;		      // Source capability index
	int dst = Config_S3K_CAP_CNT; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80030000, MEM_RWX);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst]);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_INDEX, ks.ptable[pid]->t0);
}

/*
 * Check that a memory capability can not be derived to a occupied destination index.
 */
void test_Syscall_cap_derive_memory_invalid_dest2(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 2; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80030000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_DST_OCCUPIED, ks.ptable[pid]->t0);
}

/*
 * Check that a memory capability can not be derived to its own index.
 */
void test_Syscall_cap_derive_memory_invalid_dest3(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 1; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80030000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_DST_OCCUPIED, ks.ptable[pid]->t0);
}

/*
 * Check that a memory capability can not be derived with zero size
 */
void test_Syscall_cap_derive_memory_invalid_range1(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 6; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80020000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

/*
 * Check that a memory capability can not be derived with negative size
 */
void test_Syscall_cap_derive_memory_invalid_range2(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 6; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80010000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

/*
 * Check that a memory capability's range must subset its parent's range.
 */
void test_Syscall_cap_derive_memory_invalid_range3(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 6; // Destination capability index
	cap_t cap = cap_mk_memory(0x80010000, 0x80040000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

/*
 * Check that a memory capability's range must subset its parent's range.
 */
void test_Syscall_cap_derive_memory_invalid_range4(void)
{
	int pid = 0; // Process ID
	int src = 1; // Source capability index
	int dst = 6; // Destination capability index
	cap_t cap = cap_mk_memory(0x80020000, 0x80140000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

/*
 * Check that a memory capability's permissions must subset its parent's permissions.
 */
void test_Syscall_cap_derive_memory_invalid_perm1(void)
{
	int pid = 0;  // Process ID
	int src = 1;  // Source capability index
	int dst1 = 6; // Destination capability index
	int dst2 = 7; // Another destination capability index
	cap_t cap1 = cap_mk_memory(0x80020000, 0x80040000, MEM_R);
	cap_t cap2 = cap_mk_memory(0x80020000, 0x80040000, MEM_RWX);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, dst1, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

/*
 * Check that a memory capability's permissions must subset its parent's permissions.
 */
void test_Syscall_cap_derive_memory_invalid_perm2(void)
{
	int pid = 0;  // Process ID
	int src = 1;  // Source capability index
	int dst1 = 6; // Destination capability index
	int dst2 = 7; // Another destination capability index
	cap_t cap1 = cap_mk_memory(0x80020000, 0x80040000, MEM_R);
	cap_t cap2 = cap_mk_memory(0x80020000, 0x80040000, MEM_RW);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, dst1, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
}

/*
 * Check that when deleting a PMP capability, the corresponding PMP configuration is cleared.
 */
void test_Syscall_cap_delete_pmp_valid1(void)
{
	int pid = 0; // Process ID
	int src = 0; // Source capability index
	Syscall_cap_delete(&ks, pid, src);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[src]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ptable[pid]->pmpcfg[0]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ptable[pid]->pmpaddr[0]);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
}

/*
 * Check that when moving a PMP capability, the corresponding PMP configuration is not cleared.
 */
void test_Syscall_cap_move_pmp_valid1(void)
{
	int pid = 0; // Process ID
	int src = 0; // Source capability index
	int dst = 8; // Destination capability index
	u64 cap = ks.ctable[src];
	u64 cfg = ks.ptable[pid]->pmpcfg[0];
	u64 addr = ks.ptable[pid]->pmpaddr[0];
	Syscall_cap_move(&ks, pid, src, dst);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[src]);
	TEST_ASSERT_EQUAL_UINT64(cap, ks.ctable[dst]);
	TEST_ASSERT_EQUAL_UINT64(cfg, ks.ptable[pid]->pmpcfg[0]);
	TEST_ASSERT_EQUAL_UINT64(addr, ks.ptable[pid]->pmpaddr[0]);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
}

/*
 * Check that when revoking a PMP capability, nothing happens.
 */
void test_Syscall_cap_revoke_pmp(void)
{
	int pid = 0; // Process ID
	int src = 0; // Source capability index
	u64 cap = ks.ctable[src];
	u64 cfg = ks.ptable[pid]->pmpcfg[0];
	u64 addr = ks.ptable[pid]->pmpaddr[0];
	Syscall_cap_revoke(&ks, pid, src);
	TEST_ASSERT_EQUAL_UINT64(cap, ks.ctable[src]);
	TEST_ASSERT_EQUAL_UINT64(cfg, ks.ptable[pid]->pmpcfg[0]);
	TEST_ASSERT_EQUAL_UINT64(addr, ks.ptable[pid]->pmpaddr[0]);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
}

/*
 * Derives a valid time capability.
 */
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

/*
 * Checks that revocation revokes a time capability correctly.
 */
void test_Syscall_cap_revoke_time1(void)
{
	rtc_time_set(0);
	rtc_timeout_set(0, 1000); // Reset timeout
	int pid = 0;		  // Process ID
	int src = 3;		  // Source capability index
	int dst = 8;		  // Destination capability index
	cap_t cap = cap_mk_time(0, 16);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst]);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	Syscall_cap_revoke(&ks, pid, dst);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_TRUE(check_schedule(pid, 0, 16));
	TEST_ASSERT_TRUE(check_schedule(pid, 16, S3K_SLOT_CNT));
}

/*
 * Check that time revocation a child.
 */
void test_Syscall_cap_revoke_time2(void)
{
	rtc_time_set(0);
	rtc_timeout_set(0, 1000); // Reset timeout
	int pid = 0;		  // Process ID
	int src = 3;		  // Source capability index
	int dst = 8;		  // Destination capability index
	cap_t cap = cap_mk_time(0, 16);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst]);
	Syscall_cap_derive(&ks, pid, src, dst, cap.raw);
	Syscall_cap_revoke(&ks, pid, src);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst]);
	TEST_ASSERT_TRUE(check_schedule(pid, 0, S3K_SLOT_CNT));
}

/*
 * Check that time revocation deletes all children siblings.
 */
void test_Syscall_cap_revoke_time3(void)
{
	rtc_time_set(0);
	rtc_timeout_set(0, 1000); // Reset timeout
	int pid = 0;		  // Process ID
	int src = 3;		  // Source capability index
	int dst1 = 8;		  // Destination capability index
	int dst2 = 9;		  // Destination capability index
	cap_t cap1 = cap_mk_time(0, 16);
	cap_t cap2 = cap_mk_time(16, S3K_SLOT_CNT);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst2]);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, src, dst2, cap2.raw);
	TEST_ASSERT_TRUE(check_schedule(pid, 0, 16));
	TEST_ASSERT_TRUE(check_schedule(pid, 16, S3K_SLOT_CNT));
	Syscall_cap_revoke(&ks, pid, src);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst2]);
	TEST_ASSERT_TRUE(check_schedule(pid, 0, S3K_SLOT_CNT));
}

/*
 * Check that revocation does not delete siblings.
 */
void test_Syscall_cap_revoke_time4(void)
{
	rtc_time_set(0);
	rtc_timeout_set(0, 1000); // Reset timeout
	int pid = 0;		  // Process ID
	int src = 3;		  // Source capability index
	int dst1 = 8;		  // Destination capability index
	int dst2 = 9;		  // Destination capability index
	cap_t cap1 = cap_mk_time(0, 16);
	cap_t cap2 = cap_mk_time(16, S3K_SLOT_CNT);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst2]);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, src, dst2, cap2.raw);

	// Revoke on dst1, should be a no-op
	Syscall_cap_revoke(&ks, pid, dst1);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap1.raw, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(cap2.raw, ks.ctable[dst2]);
	TEST_ASSERT_TRUE(check_schedule(pid, 0, 16));
	TEST_ASSERT_TRUE(check_schedule(pid, 16, S3K_SLOT_CNT));

	// Revoke on dst2, should be a no-op
	Syscall_cap_revoke(&ks, pid, dst2);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap1.raw, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(cap2.raw, ks.ctable[dst2]);
	TEST_ASSERT_TRUE(check_schedule(pid, 0, 16));
	TEST_ASSERT_TRUE(check_schedule(pid, 16, S3K_SLOT_CNT));
}

/*
 * Check that revocation is preempted.
 */
void test_Syscall_cap_revoke_time5(void)
{
	rtc_time_set(1);
	rtc_timeout_set(0, 0); // Reset timeout
	int pid = 0;	       // Process ID
	int src = 3;	       // Source capability index
	int dst1 = 8;	       // Destination capability index
	int dst2 = 9;	       // Destination capability index
	cap_t cap1 = cap_mk_time(0, 16);
	cap_t cap2 = cap_mk_time(16, S3K_SLOT_CNT);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst2]);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, src, dst2, cap2.raw);
	Syscall_cap_revoke(&ks, pid, src);
	TEST_ASSERT_EQUAL_UINT64(Error_PREEMPTED, ks.ptable[pid]->t0);
}

/*
 * Check that revoke deletes at least one child.
 */
void test_Syscall_cap_revoke_time6(void)
{
	// Check that revoke deletes at least one child
	rtc_time_set(1);
	rtc_timeout_set(0, 0); // Reset timeout
	int pid = 0;	       // Process ID
	int src = 3;	       // Source capability index
	int dst1 = 8;	       // Destination capability index
	int dst2 = 9;	       // Destination capability index
	cap_t cap1 = cap_mk_time(0, 16);
	cap_t cap2 = cap_mk_time(16, S3K_SLOT_CNT);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst2]);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, src, dst2, cap2.raw);
	Syscall_cap_revoke(&ks, pid, src);
	TEST_ASSERT_EQUAL_UINT64(Error_PREEMPTED, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap1.raw, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst2]);
}

/*
 * Check that revoke deletes children vertically.
 */
void test_Syscall_cap_revoke_time7(void)
{
	// Check that revoke deletes at least one child
	rtc_time_set(0);
	rtc_timeout_set(0, 100); // Reset timeout
	int pid = 0;		 // Process ID
	int src = 3;		 // Source capability index
	int dst1 = 8;		 // Destination capability index
	int dst2 = 9;		 // Destination capability index
	int dst3 = 10;		 // Destination capability index
	cap_t cap1 = cap_mk_time(0, 16);
	cap_t cap2 = cap_mk_time(16, S3K_SLOT_CNT);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst2]);
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	Syscall_cap_derive(&ks, pid, src, dst2, cap2.raw);
	Syscall_cap_derive(&ks, pid, dst2, dst3, cap2.raw);
	Syscall_cap_revoke(&ks, pid, src);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst2]);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst3]);
}

/*
 * Check that monitors can be derived correctly.
 */
void test_Syscall_cap_derive_monitor_valid1(void)
{
	int pid = 0;   // Process ID
	int src = 4;   // Source capability index
	int dst1 = 8;  // Destination capability index
	int dst2 = 9;  // Destination capability index
	int dst3 = 10; // Destination capability index
	cap_t cap1 = cap_mk_monitor(0, 8);
	cap_t cap2 = cap_mk_monitor(0, 4);
	cap_t cap3 = cap_mk_monitor(4, 8);
	TEST_ASSERT_EQUAL_UINT64(Cap_CAPTY_MONITOR,
				 Cap_get_type(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap1.raw, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(8, Cap_monitor_get_mrk(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, dst1, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(cap2.raw, ks.ctable[dst2]);
	TEST_ASSERT_EQUAL_UINT64(4, Cap_monitor_get_mrk(ks.ctable[dst1]));
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	Syscall_cap_derive(&ks, pid, dst1, dst3, cap3.raw);
	TEST_ASSERT_EQUAL_UINT64(cap3.raw, ks.ctable[dst3]);
	TEST_ASSERT_EQUAL_UINT64(8, Cap_monitor_get_mrk(ks.ctable[dst1]));
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);

	cap1.chan.mrk = 8;
	TEST_ASSERT_EQUAL_UINT64(cap1.raw, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(cap2.raw, ks.ctable[dst2]);
	TEST_ASSERT_EQUAL_UINT64(cap3.raw, ks.ctable[dst3]);
}

void test_Syscall_cap_mon_resume_suspend_valid1(void)
{
	int pid = 0; // Process ID
	int src = 4; // Source capability index
	int other_pid = 1;
	TEST_ASSERT_EQUAL_UINT64(Proc_PSF_SUSPENDED,
				 ks.ptable[other_pid]->state);
	Syscall_mon_resume(&ks, pid, src, other_pid);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ptable[other_pid]->state);
	Syscall_mon_suspend(&ks, pid, src, other_pid);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(Proc_PSF_SUSPENDED,
				 ks.ptable[other_pid]->state);
}

void test_Syscall_cap_mon_reg_valid1(void)
{
	int pid = 0; // Process ID
	int src = 4; // Source capability index
	int other_pid = 1;
	Syscall_mon_reg_write(&ks, pid, src, other_pid, 2, 0xdeadbeef);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0xdeadbeef, ks.ptable[other_pid]->sp);
	Syscall_mon_reg_read(&ks, pid, src, other_pid, 2);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0xdeadbeef, ks.ptable[pid]->a0);

	Syscall_mon_reg_write(&ks, pid, src, other_pid, 10, 0xdeadbeef);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0xdeadbeef, ks.ptable[other_pid]->a0);
	Syscall_mon_reg_read(&ks, pid, src, other_pid, 10);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0xdeadbeef, ks.ptable[pid]->a0);

	Syscall_mon_reg_write(&ks, pid, src, other_pid, 32, 0xdeadbeef);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0xdeadbeef, ks.ptable[other_pid]->tpc);
	Syscall_mon_reg_read(&ks, pid, src, other_pid, 32);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0xdeadbeef, ks.ptable[pid]->a0);
}

/*
 * Sibling channel derivation
 */
void test_Syscall_cap_derive_channel_valid1(void)
{
	int pid = 0;   // Process ID
	int src = 5;   // Source capability index
	int dst1 = 8;  // Destination capability index
	int dst2 = 9;  // Destination capability index
	int dst3 = 10; // Destination capability index
	cap_t cap1 = cap_mk_channel(0, 8);
	cap_t cap2 = cap_mk_channel(0, 4);
	cap_t cap3 = cap_mk_channel(4, 8);
	TEST_ASSERT_EQUAL_UINT64(Cap_CAPTY_CHANNEL,
				 Cap_get_type(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap1.raw, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(8, Cap_channel_get_mrk(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, dst1, dst2, cap2.raw);
	TEST_ASSERT_EQUAL_UINT64(cap2.raw, ks.ctable[dst2]);
	TEST_ASSERT_EQUAL_UINT64(4, Cap_channel_get_mrk(ks.ctable[dst1]));
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	Syscall_cap_derive(&ks, pid, dst1, dst3, cap3.raw);
	TEST_ASSERT_EQUAL_UINT64(cap3.raw, ks.ctable[dst3]);
	TEST_ASSERT_EQUAL_UINT64(8, Cap_channel_get_mrk(ks.ctable[dst1]));
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);

	cap1.chan.mrk = 8;
	TEST_ASSERT_EQUAL_UINT64(cap1.raw, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(cap2.raw, ks.ctable[dst2]);
	TEST_ASSERT_EQUAL_UINT64(cap3.raw, ks.ctable[dst3]);
}

/*
 * Yielding socket derivation
 */
void test_Syscall_cap_derive_channel_valid3(void)
{
	int pid = 0; // Process ID
	int src = 5; // Source capability index
	int dst1 = 8; // Destination capability index
	int chan = 4;
	int mode = Ipc_IPC_YIELD;
	int perm = Ipc_IPC_SCAP | Ipc_IPC_CCAP;
	int tag = 0;
	cap_t cap1 = cap_mk_socket(chan, mode, perm, tag);
	TEST_ASSERT_EQUAL_UINT64(Cap_CAPTY_CHANNEL,
				 Cap_get_type(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap1.raw, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(chan + 1, Cap_channel_get_mrk(ks.ctable[src]));
}

/*
 * No yielding socket derivation
 */
void test_Syscall_cap_derive_channel_valid4(void)
{
	int pid = 0; // Process ID
	int src = 5; // Source capability index
	int dst1 = 8; // Destination capability index
	int chan = 4;
	int mode = Ipc_IPC_NOYIELD;
	int perm = Ipc_IPC_SCAP | Ipc_IPC_CCAP;
	int tag = 0;
	cap_t cap1 = cap_mk_socket(chan, mode, perm, tag);
	TEST_ASSERT_EQUAL_UINT64(Cap_CAPTY_CHANNEL,
				 Cap_get_type(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_SUCCESS, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(cap1.raw, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(chan + 1, Cap_channel_get_mrk(ks.ctable[src]));
}

/*
 * Invalid channel range
 */
void test_Syscall_cap_derive_channel_invalid1(void)
{
	int pid = 0; // Process ID
	int src = 5; // Source capability index
	int dst1 = 8; // Destination capability index
	cap_t cap1 = cap_mk_channel(0, 20);
	TEST_ASSERT_EQUAL_UINT64(Cap_CAPTY_CHANNEL,
				 Cap_get_type(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
}

/*
 * Invalid channel range
 */
void test_Syscall_cap_derive_channel_invalid2(void)
{
	int pid = 0; // Process ID
	int src = 5; // Source capability index
	int dst1 = 8; // Destination capability index
	cap_t cap1 = cap_mk_channel(4, 4);
	TEST_ASSERT_EQUAL_UINT64(Cap_CAPTY_CHANNEL,
				 Cap_get_type(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
}

/*
 * Invalid socket tag
 */
void test_Syscall_cap_derive_channel_invalid3(void)
{
	int pid = 0; // Process ID
	int src = 5; // Source capability index
	int dst1 = 8; // Destination capability index
	int chan = 4;
	int mode = Ipc_IPC_YIELD;
	int perm = Ipc_IPC_SCAP | Ipc_IPC_CCAP;
	int tag = 1;
	cap_t cap1 = cap_mk_socket(chan, mode, perm, tag);
	TEST_ASSERT_EQUAL_UINT64(Cap_CAPTY_CHANNEL,
				 Cap_get_type(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, Cap_channel_get_mrk(ks.ctable[src]));
}

/*
 * Invalid socket tag
 */
void test_Syscall_cap_derive_channel_invalid4(void)
{
	int pid = 0; // Process ID
	int src = 5; // Source capability index
	int dst1 = 8; // Destination capability index
	int chan = 4;
	int mode = Ipc_IPC_NOYIELD;
	int perm = Ipc_IPC_SCAP | Ipc_IPC_CCAP;
	int tag = 1;
	cap_t cap1 = cap_mk_socket(chan, mode, perm, tag);
	TEST_ASSERT_EQUAL_UINT64(Cap_CAPTY_CHANNEL,
				 Cap_get_type(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, Cap_channel_get_mrk(ks.ctable[src]));
}

/*
 * Invalid socket mode.
 */
void test_Syscall_cap_derive_channel_invalid5(void)
{
	int pid = 0; // Process ID
	int src = 5; // Source capability index
	int dst1 = 8; // Destination capability index
	int chan = 4;
	int mode = 3;
	int perm = Ipc_IPC_SCAP | Ipc_IPC_CCAP;
	int tag = 0;
	cap_t cap1 = cap_mk_socket(chan, mode, perm, tag);
	TEST_ASSERT_EQUAL_UINT64(Cap_CAPTY_CHANNEL,
				 Cap_get_type(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, Cap_channel_get_mrk(ks.ctable[src]));
}


/*
 * Invalid socket channel
 */
void test_Syscall_cap_derive_channel_invalid6(void)
{
	int pid = 0; // Process ID
	int src = 5; // Source capability index
	int dst1 = 8; // Destination capability index
	int chan = 8;
	int mode = Ipc_IPC_YIELD;
	int perm = Ipc_IPC_SCAP | Ipc_IPC_CCAP;
	int tag = 0;
	cap_t cap1 = cap_mk_socket(chan, mode, perm, tag);
	TEST_ASSERT_EQUAL_UINT64(Cap_CAPTY_CHANNEL,
				 Cap_get_type(ks.ctable[src]));
	Syscall_cap_derive(&ks, pid, src, dst1, cap1.raw);
	TEST_ASSERT_EQUAL_UINT64(Error_INVALID_DERIVATION, ks.ptable[pid]->t0);
	TEST_ASSERT_EQUAL_UINT64(0, ks.ctable[dst1]);
	TEST_ASSERT_EQUAL_UINT64(0, Cap_channel_get_mrk(ks.ctable[src]));
}
