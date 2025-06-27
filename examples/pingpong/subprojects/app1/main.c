#include "s3k.h"
#include "serio.h"

int main(void)
{
	serio_printf("Client: Starting...\n");
	s3k_msg_t msg;
	msg.send_cap = false;
	msg.data[0] = 0;
	for (int i = 0; i < 100; i++) {
		s3k_reply_t reply = s3k_sock_sendrecv(3, &msg);
		if (reply.err == S3K_SUCCESS) {
			// Process the reply from app1
			serio_printf("Client: Received reply: tag=%d, data[0]=%d\n",
				     reply.tag, reply.data[0]);
			msg.data[0] = reply.data[0] + 1; // Increment the data
		} else {
			serio_printf("Client: Error %d\n", reply.err);
		}
		s3k_sleep(0); // Sleep for a while before sending the next message
	}
}
