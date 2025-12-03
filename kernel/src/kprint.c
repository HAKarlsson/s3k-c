#include "kprint.h"

#include "types.h"
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
	case Error_Success:
		kprintf("SUCCESS");
		break;
	case Error_Empty:
		kprintf("EMPTY");
		break;
	case Error_Src_empty:
		kprintf("SRC_EMPTY");
		break;
	case Error_Dst_occupied:
		kprintf("DIST_OCCUPIED");
		break;
	case Error_Invalid_index:
		kprintf("INVALID_INDEX");
		break;
	case Error_Invalid_derivation:
		kprintf("INVALID_DERIVATION");
		break;
	case Error_Invalid_monitor:
		kprintf("INVALID_MONITOR");
		break;
	case Error_Invalid_pid:
		kprintf("INVALID_PID");
		break;
	case Error_Invalid_state:
		kprintf("INVALID_STATE");
		break;
	case Error_Invalid_pmp:
		kprintf("INVALID_PMP");
		break;
	case Error_Invalid_slot:
		kprintf("INVALID_SLOT");
		break;
	case Error_Invalid_socket:
		kprintf("INVALID_SOCKET");
		break;
	case Error_Invalid_syscall:
		kprintf("INVALID_SYSCALL");
		break;
	case Error_Invalid_register:
		kprintf("INVALID_REGISTER");
		break;
	case Error_Invalid_capability:
		kprintf("INVALID_CAPABILITY");
		break;
	case Error_No_receiver:
		kprintf("NO_RECEIVER");
		break;
	case Error_Preempted:
		kprintf("PREEMPTED");
		break;
	case Error_Timeout:
		kprintf("TIMEOUT");
		break;
	case Error_Suspended:
		kprintf("SUSPENDED");
		break;
	case Error_Continue:
		kprintf("CONTINUE");
		break;
	default:
		kprintf("UNKNOWN");
	}
}

const char *rwx2str(rwx_t rwx)
{
	switch (rwx) {
	case MEM_RWX:
		return "RWX";
	case MEM_RW:
		return "RW-";
	case MEM_RX:
		return "R-X";
	case MEM_R:
		return "R--";
	case MEM_WX:
		return "-WX";
	case MEM_W:
		return "-W-";
	case MEM_X:
		return "--X";
	default:
		return "---";
	}
}

void cap_print(cap_t cap)
{
	switch (Cap_get_type(cap)) {
	case Cap_Capty_none:
		kprintf("NONE{}");
		break;
	case Cap_Capty_time:
		kprintf("TIME{bgn=%d,end=%d,mrk=%d}", Cap_time_get_low(cap),
			Cap_time_get_upp(cap), Cap_time_get_mrk(cap));
		break;

	case Cap_Capty_memory:
		u64 bgn = Util_tag_block_to_addr(Cap_memory_get_tag(cap),
						 Cap_memory_get_low(cap));
		u64 end = Util_tag_block_to_addr(Cap_memory_get_tag(cap),
						 Cap_memory_get_upp(cap));
		u64 mrk = Util_tag_block_to_addr(Cap_memory_get_tag(cap),
						 Cap_memory_get_mrk(cap));
		kprintf("MEMORY{bgn=0x%X,end=0x%X,mrk=0x%X,rwx=%s,lck=%x}", bgn,
			end, mrk, rwx2str(Cap_memory_get_rwx(cap)),
			Cap_memory_get_lck(cap));
		break;
	case Cap_Capty_pmp:
		word_t pmp_base
		    = Util_pmp_napot_decode_base(Cap_pmp_get_addr(cap));
		word_t pmp_size
		    = Util_pmp_napot_decode_size(Cap_pmp_get_addr(cap));
		kprintf("PMP{bgn=0x%X,end=0x%X,rwx=%s,used=%d,slot=%d}",
			pmp_base, pmp_base + pmp_size,
			rwx2str(Cap_pmp_get_rwx(cap)), Cap_pmp_get_used(cap),
			Cap_pmp_get_slot(cap));
		break;
	case Cap_Capty_monitor:
		kprintf("MONITOR{bgn=%d,end=%d,mrk=%d}",
			Cap_monitor_get_low(cap), Cap_monitor_get_upp(cap),
			Cap_monitor_get_mrk(cap));
		break;
	case Cap_Capty_channel:
		kprintf("CHANNEL{bgn=%d,end=%d,mrk=%d}",
			Cap_channel_get_low(cap), Cap_channel_get_upp(cap),
			Cap_channel_get_mrk(cap));
		break;
	case Cap_Capty_socket:
		kprintf("SOCKET{chan=%d,tag=%d,perm=%d,mode=%d}",
			Cap_socket_get_chan(cap), Cap_socket_get_tag(cap),
			Cap_socket_get_perm(cap), Cap_socket_get_mode(cap));
		break;

	default:
		kprintf("UNKNOWN{raw=0x%X}", cap);
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
