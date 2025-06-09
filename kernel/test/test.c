#include "cap/pmp.h"
#include "cap/table.h"
#include "cap/util.h"
#include "kinit.h"
#include "libkernel.h"
#include "pmp.h"
#include "proc.h"
#include "sched.h"
#include "types.h"
#include "unity.h"

extern struct Types_kstate ks;

void setUp(void)
{
	kinit_vregs();
	cap_t init_caps[5];
	cap_mk_pmp(&init_caps[0], pmp_napot_encode(0x80010000, 0x10000), MEM_RWX);
	cap_mk_memory(&init_caps[1], 0x80010000, 0x80100000, MEM_RWX);
	cap_mk_memory(&init_caps[2], 0x10000000, 0x10010000, MEM_RW);
	cap_mk_time(&init_caps[3], 0, S3K_SLOT_CNT);
	cap_mk_monitor(&init_caps[4], 0, S3K_PROC_CNT);
	kinit_ctable(init_caps, ARRAY_SIZE(init_caps));
	sched_init();
	proc_init(0x80010000);
}

void tearDown(void)
{
	// clean stuff up here
	ks = (struct Types_kstate){0};
}

void test_mk_memory(void)
{
	cap_t cap = {.raw = 0x123456789abcdef0ull };
	cap.type = CAPTY_MEMORY;
	uint64_t ccap = convert_cap(cap);
	TEST_ASSERT_EQUAL_UINT64(cap.mem.rwx, Cap_memory_get_rwx(ccap));
	TEST_ASSERT_EQUAL_UINT64(cap.mem.bgn, Cap_memory_get_bgn(ccap));
	TEST_ASSERT_EQUAL_UINT64(cap.mem.end, Cap_memory_get_end(ccap));
	TEST_ASSERT_EQUAL_UINT64(cap.mem.tag, Cap_memory_get_tag(ccap));
	TEST_ASSERT_EQUAL_UINT64(cap.mem.lck, Cap_memory_get_lck(ccap));
	TEST_ASSERT_EQUAL_UINT64(cap.mem.mrk, Cap_memory_get_mrk(ccap));
}

void test_mk_time(void)
{
	cap_t cap = {.raw = 0x123456789abcdef0ull };
	cap.type = CAPTY_TIME;
	uint64_t ccap = convert_cap(cap);
	TEST_ASSERT_EQUAL_UINT64(cap.type, Cap_get_type(ccap));
	TEST_ASSERT_EQUAL_UINT64(cap.time.bgn, Cap_time_get_bgn(ccap));
	TEST_ASSERT_EQUAL_UINT64(cap.time.end, Cap_time_get_end(ccap));
	TEST_ASSERT_EQUAL_UINT64(cap.time.mrk, Cap_time_get_mrk(ccap));
}

void test_mk_monitor(void)
{
	cap_t cap = {.raw = 0x123456789abcdef0ull };
	cap.type = CAPTY_MONITOR;
	uint64_t ccap = convert_cap(cap);
	TEST_ASSERT_EQUAL_UINT64(cap.type, Cap_get_type(ccap));
	TEST_ASSERT_EQUAL_UINT64(cap.mon.bgn, Cap_monitor_get_bgn(ccap));
	TEST_ASSERT_EQUAL_UINT64(cap.mon.end, Cap_monitor_get_end(ccap));
	TEST_ASSERT_EQUAL_UINT64(cap.mon.mrk, Cap_monitor_get_mrk(ccap));
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
