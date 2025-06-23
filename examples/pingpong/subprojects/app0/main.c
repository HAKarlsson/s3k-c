#include "s3k.h"
#include "serio.h"

#include <stdint.h>

#define APP0_PID 0
#define APP1_PID 1

void setup_uart(uint64_t uart_idx)
{
	uint64_t uart_addr = s3k_napot_encode(UART_BASE_ADDR, 0x8);
	// Derive a PMP capability for accessing UART
	s3k_br_cap_derive(CAP_UART, uart_idx,
			  s3k_mk_pmp(uart_addr, S3K_MEM_RW));
	// Load the derive PMP capability to PMP configuration
	s3k_br_pmp_load(uart_idx, 1);
	// Synchronize PMP unit (hardware) with PMP configuration
	// false => not full synchronization.
	s3k_sync_mem();
}

void setup_app1(uint64_t tmp)
{
	uint64_t uart_addr = s3k_napot_encode(UART_BASE_ADDR, 0x8);
	uint64_t app1_addr = s3k_napot_encode(0x80020000, 0x10000);

	// Derive a PMP capability for app1 main memory
	s3k_br_cap_derive(CAP_RAM, tmp, s3k_mk_pmp(app1_addr, S3K_MEM_RWX));
	s3k_br_mon_cap_move(CAP_MONITOR, APP0_PID, tmp, APP1_PID, 0);
	s3k_br_mon_pmp_load(CAP_MONITOR, APP1_PID, 0, 0);

	// Derive a PMP capability for uart
	s3k_br_cap_derive(CAP_UART, tmp, s3k_mk_pmp(uart_addr, S3K_MEM_RW));
	s3k_br_mon_cap_move(CAP_MONITOR, APP0_PID, tmp, APP1_PID, 1);
	s3k_br_mon_pmp_load(CAP_MONITOR, APP1_PID, 1, 1);

	// derive a time slice capability
	s3k_br_cap_derive(CAP_TIME, tmp, s3k_mk_time(0, 16));
	s3k_br_mon_cap_move(CAP_MONITOR, APP0_PID, tmp, APP1_PID, 2);

	// Write start PC of app1 to PC
	s3k_br_mon_reg_write(CAP_MONITOR, APP1_PID, S3K_REG_PC, 0x80020000);
}

void setup_ipc(uint64_t server, uint64_t tmp)
{
	int err;
	int channel = 0;
	int mode = S3K_IPC_NOYIELD;
	int perm = 0;

	// Create two IPC capabilities for server and client
	s3k_cap_t server_cap = s3k_mk_socket(channel, mode, perm, 0);
	s3k_cap_t client_cap = s3k_mk_socket(channel, mode, perm, 1);
	err = s3k_br_cap_derive(CAP_CHANNEL, server, server_cap);
	while (err) {
		serio_printf("Failed to derive server capability: %d\n", err);
		s3k_sleep(1);
	}
	err = s3k_br_cap_derive(server, tmp, client_cap);
	while (err) {
		serio_printf("Failed to derive client capability: %d\n", err);
		s3k_sleep(1);
	}

	// Send client capability to app1
	s3k_br_mon_cap_move(CAP_MONITOR, APP0_PID, tmp, APP1_PID, 3);
	while (err) {
		serio_printf("Failed to move client capability: %d\n", err);
		s3k_sleep(1);
	}
}

int main(void)
{
	// Setup UART access
	setup_uart(10);

	// Setup app1 capabilities and PC
	setup_app1(11);

	// Setup IPC channel between app0 and app1
	setup_ipc(12, 11);

	// Start app1
	s3k_br_mon_resume(CAP_MONITOR, APP1_PID);

	s3k_msg_t msg;
	while (1) {
		s3k_reply_t reply = s3k_br_sock_sendrecv(12, &msg);
		if (reply.err == S3K_SUCCESS) {
			// Process the reply from app1
			serio_printf("App0: Received reply: tag=%d, data[0]=%d\n",
				     reply.tag, reply.data[0]);
			msg.data[0] = reply.data[0] + 1; // Increment the data
		} else {
			serio_printf("App0: Error: %d\n", reply.err);
		}
	}
}
