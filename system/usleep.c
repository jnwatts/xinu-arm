/**
 * @file     usleep.c
 * @provides usleep.
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <kernel.h>
#include <stddef.h>
#include <platform.h>
#include <interrupt.h>
#include <clock.h>
#include <queue.h>
#include "conf.h"

extern void clkdelay(ulong cycles);

#if RTCLOCK

/**
 * Sleep for N microseconds
 */
void usleep(ulong usec)
{
	clkdelay(usec);
}


/**
 * Sleep for N miliseconds
 */
void mdelay(unsigned long msec)
{
	while (msec--)
		usleep(1000);
}

#endif                          /* RTCLOCK */
