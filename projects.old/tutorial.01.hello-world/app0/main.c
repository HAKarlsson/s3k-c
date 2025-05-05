#include "../../tutorial-commons/utils.h"
#include "altc/altio.h"
#include "s3k/s3k.h"

int main(void)
{
	// Setup UART access
	setup_uart();

	// Write hello world.
	alt_printf("Hello, world\n");
}
