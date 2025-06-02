#include "s3k.h"
#include "serio.h"

int main(void)
{
	while (1) {
		serio_putstr("Hello from app1!\n");
		s3k_sleep(0);
	}
}
