#include "kprint.h"

#include "cap/util.h"
#include "pmp.h"
#include "types.h"

#include <stdarg.h>
#include <stdbool.h>

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
	case MEM_W:
		return "-W-";
	case MEM_X:
		return "--X";
	default:
		return "---";
	}
}

void cap_print(const cap_t *cap)
{
	switch (cap->type) {
	case CAPTY_NONE:
		kprintf("NONE{}");
		break;
	case CAPTY_TIME:
		kprintf("TIME{bgn=%d,end=%d,mrk=%d}", cap->time.bgn,
			cap->time.end, cap->time.mrk);
		break;
	case CAPTY_MEMORY: {
		uint64_t bgn = tag_block_to_addr(cap->mem.tag, cap->mem.bgn);
		uint64_t end = tag_block_to_addr(cap->mem.tag, cap->mem.end);
		uint64_t mrk = tag_block_to_addr(cap->mem.tag, cap->mem.mrk);
		kprintf("MEMORY{bgn=0x%X,end=0x%X,mrk=0x%X,rwx=%s,lck=%x}", bgn,
			end, mrk, rwx2str(cap->mem.rwx), cap->mem.lck);
	} break;
	case CAPTY_PMP: {
		word_t pmp_addr = cap->raw >> 16;
		word_t pmp_base = pmp_napot_decode_base(pmp_addr);
		word_t pmp_size = pmp_napot_decode_size(pmp_addr);
		kprintf("PMP{bgn=0x%X,end=0x%X,rwx=%s,used=%d,slot=%d}",
			pmp_base, pmp_base + pmp_size, rwx2str(cap->mem.rwx),
			cap->pmp.used, cap->pmp.slot);
	} break;
	case CAPTY_MONITOR:
		kprintf("MONITOR{bgn=%d,end=%d,mrk=%d}", cap->mon.bgn,
			cap->mon.end, cap->mon.mrk);
		break;
	case CAPTY_CHANNEL:
		kprintf("CHANNEL{bgn=%d,end=%d,mrk=%d}", cap->chan.bgn,
			cap->chan.end, cap->chan.mrk);
		break;
	case CAPTY_SOCKET:
		kprintf("SOCKET{chan=%d,tag=%d,perm=%d,mode=%d}",
			cap->sock.chan, cap->sock.tag, cap->sock.perm,
			cap->sock.mode);
		break;
	default:
		kprintf("UNKNOWN{raw=0x%X}", cap->raw);
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
