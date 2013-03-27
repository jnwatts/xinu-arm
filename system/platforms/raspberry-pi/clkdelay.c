#include "bcm2835.h"
#include "stddef.h"
#include "interrupt.h"
#include "vic.h"
#include "conf.h"

void clkdelay(ulong cycles)
{
	ulong endtime;
	long diff;

	endtime = TIMER_CLO + cycles;

	do {
		ulong now = TIMER_CLO + cycles;
		diff = endtime - now;
	} while (diff >= 0);
}

