#include "serio.h"
#include "serio/ns16550a.h"

int serio_getchar(void)
{
	return serio_ns16550a_getchar((void *)UART_BASE_ADDR);
}

int serio_putchar(int c)
{
	return serio_ns16550a_putchar(c, (void *)UART_BASE_ADDR);
}
