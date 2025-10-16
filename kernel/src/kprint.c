#include "kprint.h"

#include "cap/util.h"
#include "libkernel.h"

#include <stdarg.h>

extern void kputc(char c);

void kputstr(const char *s)
{
	while (*s != '\0') {
		kputc(*s++);
	}
}

void kputs(const char *s)
{
	kputstr(s);
	kputc('\n');
}

void _print_hex(unsigned long long x)
{
	if (!x) {
		kputc('0');
		return;
	}

	char hex[16];
	int i = 0;
	while (x) {
		unsigned int h = x % 16;
		if (h < 10)
			hex[i++] = '0' + h;
		else
			hex[i++] = 'A' + h - 10;
		x >>= 4;
	}
	while (i) {
		kputc(hex[--i]);
	}
}

void _print_dec(unsigned long long x, bool neg)
{
	if (!x) {
		kputc('0');
		return;
	}

	char dec[32];
	int i = 0;
	while (x) {
		dec[i++] = '0' + (x % 10);
		x /= 10;
	}
	if (neg)
		dec[i++] = '-';

	while (i) {
		kputc(dec[--i]);
	}
}

void _print_err(err_t e)
{
	switch (e) {
	case Error_SUCCESS:
		kprintf("SUCCESS");
		break;
	case Error_EMPTY:
		kprintf("Error_EMPTY");
		break;
	case Error_SRC_EMPTY:
		kprintf("Error_SRC_EMPTY");
		break;
	case Error_DST_OCCUPIED:
		kprintf("Error_DIST_OCCUPIED");
		break;
	case Error_INVALID_INDEX:
		kprintf("Error_INVALID_INDEX");
		break;
	case Error_INVALID_DERIVATION:
		kprintf("Error_INVALID_DERIVATION");
		break;
	case Error_INVALID_MONITOR:
		kprintf("Error_INVALID_MONITOR");
		break;
	case Error_INVALID_PID:
		kprintf("Error_INVALID_PID");
		break;
	case Error_INVALID_STATE:
		kprintf("Error_INVALID_STATE");
		break;
	case Error_INVALID_PMP:
		kprintf("Error_INVALID_PMP");
		break;
	case Error_INVALID_SLOT:
		kprintf("Error_INVALID_SLOT");
		break;
	case Error_INVALID_SOCKET:
		kprintf("Error_INVALID_SOCKET");
		break;
	case Error_INVALID_SYSCALL:
		kprintf("Error_INVALID_SYSCALL");
		break;
	case Error_INVALID_REGISTER:
		kprintf("Error_INVALID_REGISTER");
		break;
	case Error_INVALID_CAPABILITY:
		kprintf("Error_INVALID_CAPABILITY");
		break;
	case Error_NO_RECEIVER:
		kprintf("Error_NO_RECEIVER");
		break;
	case Error_PREEMPTED:
		kprintf("Error_PREEMPTED");
		break;
	case Error_TIMEOUT:
		kprintf("Error_TIMEOUT");
		break;
	case Error_SUSPENDED:
		kprintf("Error_SUSPENDED");
		break;
	case Error_CONTINUE:
		kprintf("CONTINUE");
		break;
	default:
		kprintf("UNKNOWN");
	}
}

void kprintf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	while (*fmt != '\0') {
		if (*fmt != '%') {
			kputc(*fmt);
			fmt++;
			continue;
		}
		fmt++;
		if (*fmt == '\0')
			break;
		switch (*fmt) {
		case 'x': {
			unsigned int x = va_arg(ap, unsigned int);
			_print_hex(x);
		} break;
		case 'X': {
			unsigned long long x = va_arg(ap, unsigned long long);
			_print_hex(x);
		} break;
		case 'u': {
			unsigned int x = va_arg(ap, unsigned int);
			_print_dec(x, false);
		} break;
		case 'U': {
			unsigned long long x = va_arg(ap, unsigned long long);
			_print_dec(x, false);
		} break;
		case 'd': {
			int x = va_arg(ap, unsigned int);
			bool neg = x < 0;
			_print_dec(neg ? -x : x, neg);
		} break;
		case 'D': {
			long long x = va_arg(ap, unsigned long long);
			bool neg = x < 0;
			_print_dec(neg ? -x : x, neg);
		} break;
		case 'c': {
			char c = va_arg(ap, int);
			kputc(c);
		} break;
		case 'C': {
			cap_t c = va_arg(ap, cap_t);
			cap_print(c);
		} break;
		case 'e': {
			err_t e = va_arg(ap, unsigned int);
			_print_err(e);
		} break;
		case 's': {
			char *s = va_arg(ap, char *);
			kputstr(s);
		} break;
		default:
			kputc(*fmt);
		}
		fmt++;
	}
	va_end(ap);
}
