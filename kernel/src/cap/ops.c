#include "cap/ops.h"

#include "preempt.h"
#include "types.h"

typedef struct Types_kstate *(*revoke_handler)(struct Types_kstate *, u64);

static const revoke_handler revoke_handlers[CAPTY_COUNT] = {
    NULL,
    Cap_ops_revoke_time,
    Cap_ops_revoke_memory,
    Cap_ops_revoke_pmp,
    Cap_ops_revoke_monitor,
    Cap_ops_revoke_channel,
    Cap_ops_revoke_socket,
};

struct Types_kstate *Cap_ops_revoke(struct Types_kstate *ks, u64 parent)
{
	u64 pcap = ks->ctable[parent];
	u64 type = Cap_get_type(pcap);
	if (type == Cap_CAPTY_NONE) {
		ks->errcode = Error_EMPTY;
		return ks;
	}
	do {
		revoke_handlers[type](ks, parent);
	} while (ks->errcode == Error_CONTINUE && !preempt());

	if (ks->errcode == Error_CONTINUE) {
		ks->errcode = Error_PREEMPTED;
	}
	return ks;
}