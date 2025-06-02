#include "rtc.h"

#include "libkernel.h"

extern volatile uint64_t __mtime[];
extern volatile uint64_t __mtimecmp[];

uint64_t rtc_time_get(void)
{
	return __mtime[0];
}

u64 Machine_time_read(struct Machine_state *mc)
{
	return rtc_time_get();
}

void rtc_time_set(uint64_t time)
{
	__mtime[0] = time;
}

uint64_t rtc_timeout_get(uint64_t hart)
{
	return __mtimecmp[hart];
}

void rtc_timeout_set(uint64_t hart, uint64_t time)
{
	__mtimecmp[hart] = time;
}
