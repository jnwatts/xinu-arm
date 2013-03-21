#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <gpio.h>
#include "../system/platforms/raspberry-pi/gpio.h"

#define GPIO_LED_ACT 16

void ledOn()
{
	GPIOMODE(GPIO_LED_ACT, FSEL_OUTPUT);
	GPIOCLR(GPIO_LED_ACT);
	return 0;
}

void ledOff()
{
	GPIOMODE(GPIO_LED_ACT, FSEL_OUTPUT);
	GPIOSET(GPIO_LED_ACT);
	return 0;
}
