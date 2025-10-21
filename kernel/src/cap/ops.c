#include "cap/ops.h"

#include "preempt.h"

typedef void (*revoke_handler)(u64);

static const revoke_handler revoke_handlers[CAPTY_COUNT] = {
    NULL,
    Cap_ops_revoke_time,
    Cap_ops_revoke_memory,
    Cap_ops_revoke_pmp,
    Cap_ops_revoke_monitor,
    Cap_ops_revoke_channel,
    Cap_ops_revoke_socket,
};

void Cap_ops_revoke(u64 parent)
{
	u64 pcap = Kernel_ks->ctable[parent];
	u64 type = Cap_get_type(pcap);
	if (type == Cap_CAPTY_NONE) {
		Kernel_ks->errcode = Error_EMPTY;
		return;
	}
	do {
		revoke_handlers[type](parent);
	} while (Kernel_ks->errcode == Error_CONTINUE && !preempt());

	if (Kernel_ks->errcode == Error_CONTINUE) {
		Kernel_ks->errcode = Error_PREEMPTED;
	}
	return;
}