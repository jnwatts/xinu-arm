#include "bcm2835.h"
#include "stddef.h"
#include "interrupt.h"
#include "vic.h"
#include "conf.h"

void clkdelay(ulong cycles)
{
	ulong endtime = TIMER_CLO + cycles;
	while (TIMER_CLO < endtime);
}

