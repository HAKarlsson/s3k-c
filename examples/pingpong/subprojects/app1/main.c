#include "s3k.h"
#include "serio.h"

int main(void)
{
	s3k_msg_t msg;
	msg.send_cap = false;
	msg.data[0] = 0;
	while (1) {
		s3k_reply_t reply = s3k_br_sock_sendrecv(3, &msg);
		if (reply.err == S3K_SUCCESS) {
			// Process the reply from app1
			serio_printf("App1: Received reply: tag=%d, data[0]=%d\n",
				     reply.tag, reply.data[0]);
			msg.data[0] = reply.data[0] + 1; // Increment the data
		} else {
			serio_printf("App1: Error %d\n", reply.err);
		}
		s3k_sleep(1000); // Sleep for a while before sending the next message
	}
}
