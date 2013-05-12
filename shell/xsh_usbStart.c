/*
 * @file     xsh_date.c
 * @provides xsh_date
 *
 * $Id: xsh_date.c 2108 2009-10-29 05:07:39Z brylow $
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <usb.h>

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
 #define USB_KBD_BUFFER_LEN	0x20
#define USB_KBD_NUMLOCK		(1 << 0)
#define USB_KBD_CAPSLOCK	(1 << 1)
#define USB_KBD_SCROLLLOCK	(1 << 2)
#define USB_KBD_CTRL		(1 << 3)

#define USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

struct usb_kbd_pdata {
	u32	repeat_delay;

	u32	usb_in_pointer;
	u32	usb_out_pointer;
	u8		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	u8		new[8];
	u8		old[8];

	u8		flags;
};

static void usb_kbd_setled2(struct usb_device *dev)
{
	struct usb_interface *iface = &dev->config.if_desc[0];
	struct usb_kbd_pdata *data = dev->privptr;
	u32 leds = data->flags & USB_KBD_LEDMASK;

	usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.bInterfaceNumber, (void *)&leds, 1, 0);
}
/**
 * Shell command (usbStart).
 * @param nargs  number of arguments in args array
 * @param args   array of arguments
 * @return OK for success, SYSERR for syntax error
 */
shellcmd xsh_usbStart(int nargs, char *args[])
{
    /* Check for correct number of arguments */
    if (nargs > 1)
    {
        fprintf(stderr, "%s: too many arguments\n", args[0]);
        return SYSERR;
    }
    if (nargs == 1)
    {
        if(usb_init() == 0)
			kprintf("USB Started Successfully\n\r");
		else
			kprintf("USB Failed to start\r\n");
		
    }

    return OK;
}

