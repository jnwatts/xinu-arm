/*
 * @file     xsh_usb.c
 * @provides xsh_usb
 *
 * $Id: xsh_usb.c 2108 2009-10-29 05:07:39Z brylow $
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <usb.h>
#include <part.h>
#include <string.h>
#include <unaligned/generic.h>
#include <unaligned/le_byteshift.h>
#include <ctype.h>
#define CMD_RET_USAGE -1
#define CONFIG_USB_STORAGE

/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Most of this source has been derived from the Linux USB
 * project.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <common2.h>
#include <stdlib.h>
#include <stddef.h>
#include <asm/types.h>
#include <usb.h>

#ifdef CONFIG_USB_STORAGE
static int usb_stor_curr_dev = -1; /* current device */
#endif
#ifdef CONFIG_USB_HOST_ETHER
static int usb_ether_curr_dev = -1; /* current ethernet device */
#endif
/* some display routines (info command) */
static char *usb_get_class_desc(unsigned char dclass)
{
	switch (dclass) {
	case USB_CLASS_PER_INTERFACE:
		return "See Interface";
	case USB_CLASS_AUDIO:
		return "Audio";
	case USB_CLASS_COMM:
		return "Communication";
	case USB_CLASS_HID:
		return "Human Interface";
	case USB_CLASS_PRINTER:
		return "Printer";
	case USB_CLASS_MASS_STORAGE:
		return "Mass Storage";
	case USB_CLASS_HUB:
		return "Hub";
	case USB_CLASS_DATA:
		return "CDC Data";
	case USB_CLASS_VENDOR_SPEC:
		return "Vendor specific";
	default:
		return "";
	}
}

static void usb_display_class_sub(unsigned char dclass, unsigned char subclass,
				  unsigned char proto)
{
	switch (dclass) {
	case USB_CLASS_PER_INTERFACE:
		printf("See Interface");
		break;
	case USB_CLASS_HID:
		printf("Human Interface, Subclass: ");
		switch (subclass) {
		case USB_SUB_HID_NONE:
			printf("None");
			break;
		case USB_SUB_HID_BOOT:
			printf("Boot ");
			switch (proto) {
			case USB_PROT_HID_NONE:
				printf("None");
				break;
			case USB_PROT_HID_KEYBOARD:
				printf("Keyboard");
				break;
			case USB_PROT_HID_MOUSE:
				printf("Mouse");
				break;
			default:
				printf("reserved");
				break;
			}
			break;
		default:
			printf("reserved");
			break;
		}
		break;
	case USB_CLASS_MASS_STORAGE:
		printf("Mass Storage, ");
		switch (subclass) {
		case US_SC_RBC:
			printf("RBC ");
			break;
		case US_SC_8020:
			printf("SFF-8020i (ATAPI)");
			break;
		case US_SC_QIC:
			printf("QIC-157 (Tape)");
			break;
		case US_SC_UFI:
			printf("UFI");
			break;
		case US_SC_8070:
			printf("SFF-8070");
			break;
		case US_SC_SCSI:
			printf("Transp. SCSI");
			break;
		default:
			printf("reserved");
			break;
		}
		printf(", ");
		switch (proto) {
		case US_PR_CB:
			printf("Command/Bulk");
			break;
		case US_PR_CBI:
			printf("Command/Bulk/Int");
			break;
		case US_PR_BULK:
			printf("Bulk only");
			break;
		default:
			printf("reserved");
			break;
		}
		break;
	default:
		printf("%s", usb_get_class_desc(dclass));
		break;
	}
}

static void usb_display_string(struct usb_device *dev, int index)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buffer, 256);

	if (index != 0) {
		if (usb_string(dev, index, &buffer[0], 256) > 0)
			printf("String: \"%s\"", buffer);
	}
}

static void usb_display_desc(struct usb_device *dev)
{
	if (dev->descriptor.bDescriptorType == USB_DT_DEVICE) {
		printf("%d: %s,  USB Revision %x.%x\n", dev->devnum,
		usb_get_class_desc(dev->config.if_desc[0].desc.bInterfaceClass),
				   (dev->descriptor.bcdUSB>>8) & 0xff,
				   dev->descriptor.bcdUSB & 0xff);

		if (strlen(dev->mf) || strlen(dev->prod) ||
		    strlen(dev->serial))
			printf(" - %s %s %s\n", dev->mf, dev->prod,
				dev->serial);
		if (dev->descriptor.bDeviceClass) {
			printf(" - Class: ");
			usb_display_class_sub(dev->descriptor.bDeviceClass,
					      dev->descriptor.bDeviceSubClass,
					      dev->descriptor.bDeviceProtocol);
			printf("\n");
		} else {
			printf(" - Class: (from Interface) %s\n",
			       usb_get_class_desc(
				dev->config.if_desc[0].desc.bInterfaceClass));
		}
		printf(" - PacketSize: %d  Configurations: %d\n",
			dev->descriptor.bMaxPacketSize0,
			dev->descriptor.bNumConfigurations);
		printf(" - Vendor: 0x%04x  Product 0x%04x Version %d.%d\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct,
			(dev->descriptor.bcdDevice>>8) & 0xff,
			dev->descriptor.bcdDevice & 0xff);
	}

}

static void usb_display_conf_desc(struct usb_configuration_descriptor *config,
				  struct usb_device *dev)
{
	printf("   Configuration: %d\n", config->bConfigurationValue);
	printf("   - Interfaces: %d %s%s%dmA\n", config->bNumInterfaces,
	       (config->bmAttributes & 0x40) ? "Self Powered " : "Bus Powered ",
	       (config->bmAttributes & 0x20) ? "Remote Wakeup " : "",
		config->bMaxPower*2);
	if (config->iConfiguration) {
		printf("   - ");
		usb_display_string(dev, config->iConfiguration);
		printf("\n");
	}
}

void help(void)
{
    printf("USB sub-system\n"
    		"\tusb start - start (scan) USB controller\n"
    		"\tusb reset - reset (rescan) USB controller\n"
    		"\tusb stop [f] - stop USB [f]=force stop\n"
    		"\tusb tree - show USB device tree\n"
    		"\tusb info [dev] - show available USB devices\n"
    		"\tusb storage - show details of USB storage devices\n"
    		"\tusb dev [dev] - show or set current USB storage device\n"
    		"\tusb part [dev] - print partition table of one or all USB storage"
    		" devices\n"
    		"\tusb read addr blk# cnt - read `cnt' blocks starting at block `blk#'\n"
    		"\t\tto memory address `addr'\n"
    		"\tusb write addr blk# cnt - write `cnt' blocks starting at block `blk#'\n"
    		"\t\tfrom memory address `addr'\r\n");
}

static void usb_display_if_desc(struct usb_interface_descriptor *ifdesc,
				struct usb_device *dev)
{
	printf("     Interface: %d\n", ifdesc->bInterfaceNumber);
	printf("     - Alternate Setting %d, Endpoints: %d\n",
		ifdesc->bAlternateSetting, ifdesc->bNumEndpoints);
	printf("     - Class ");
	usb_display_class_sub(ifdesc->bInterfaceClass,
		ifdesc->bInterfaceSubClass, ifdesc->bInterfaceProtocol);
	printf("\n");
	if (ifdesc->iInterface) {
		printf("     - ");
		usb_display_string(dev, ifdesc->iInterface);
		printf("\n");
	}
}

static void usb_display_ep_desc(struct usb_endpoint_descriptor *epdesc)
{
	printf("     - Endpoint %d %s ", epdesc->bEndpointAddress & 0xf,
		(epdesc->bEndpointAddress & 0x80) ? "In" : "Out");
	switch ((epdesc->bmAttributes & 0x03)) {
	case 0:
		printf("Control");
		break;
	case 1:
		printf("Isochronous");
		break;
	case 2:
		printf("Bulk");
		break;
	case 3:
		printf("Interrupt");
		break;
	}
	printf(" MaxPacket %d", get_unaligned(&epdesc->wMaxPacketSize));
	if ((epdesc->bmAttributes & 0x03) == 0x3)
		printf(" Interval %dms", epdesc->bInterval);
	printf("\n");
}

/* main routine to diasplay the configs, interfaces and endpoints */
static void usb_display_config(struct usb_device *dev)
{
	struct usb_config *config;
	struct usb_interface *ifdesc;
	struct usb_endpoint_descriptor *epdesc;
	int i, ii;

	config = &dev->config;
	usb_display_conf_desc(&config->desc, dev);
	for (i = 0; i < config->no_of_if; i++) {
		ifdesc = &config->if_desc[i];
		usb_display_if_desc(&ifdesc->desc, dev);
		for (ii = 0; ii < ifdesc->no_of_ep; ii++) {
			epdesc = &ifdesc->ep_desc[ii];
			usb_display_ep_desc(epdesc);
		}
	}
	printf("\n");
}

static inline char *portspeed(int speed)
{
	if (speed == USB_SPEED_HIGH)
		return "480 Mb/s";
	else if (speed == USB_SPEED_LOW)
		return "1.5 Mb/s";
	else
		return "12 Mb/s";
}

/* shows the device tree recursively */
static void usb_show_tree_graph(struct usb_device *dev, char *pre)
{
	int i, index;
	int has_child, last_child;

	index = strlen(pre);
	printf(" %s", pre);
	/* check if the device has connected children */
	has_child = 0;
	for (i = 0; i < dev->maxchild; i++) {
		if (dev->children[i] != NULL)
			has_child = 1;
	}
	/* check if we are the last one */
	last_child = 1;
	if (dev->parent != NULL) {
		for (i = 0; i < dev->parent->maxchild; i++) {
			/* search for children */
			if (dev->parent->children[i] == dev) {
				/* found our pointer, see if we have a
				 * little sister
				 */
				while (i++ < dev->parent->maxchild) {
					if (dev->parent->children[i] != NULL) {
						/* found a sister */
						last_child = 0;
						break;
					} /* if */
				} /* while */
			} /* device found */
		} /* for all children of the parent */
		printf("\b+-");
		/* correct last child */
		if (last_child)
			pre[index-1] = ' ';
	} /* if not root hub */
	else
		printf(" ");
	printf("%d ", dev->devnum);
	pre[index++] = ' ';
	pre[index++] = has_child ? '|' : ' ';
	pre[index] = 0;
	printf(" %s (%s, %dmA)\n", usb_get_class_desc(
					dev->config.if_desc[0].desc.bInterfaceClass),
					portspeed(dev->speed),
					dev->config.desc.bMaxPower * 2);
	if (strlen(dev->mf) || strlen(dev->prod) || strlen(dev->serial))
		printf(" %s  %s %s %s\n", pre, dev->mf, dev->prod, dev->serial);
	printf(" %s\n", pre);
	if (dev->maxchild > 0) {
		for (i = 0; i < dev->maxchild; i++) {
			if (dev->children[i] != NULL) {
				usb_show_tree_graph(dev->children[i], pre);
				pre[index] = 0;
			}
		}
	}
}

/* main routine for the tree command */
static void usb_show_tree(struct usb_device *dev)
{
	char preamble[32];

	memset(preamble, 0, 32);
	usb_show_tree_graph(dev, &preamble[0]);
}

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
	unsigned long result = 0,value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
	    ? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

long simple_strtol(const char *cp,char **endp,unsigned int base)
{
	if(*cp=='-')
		return -simple_strtoul(cp+1,endp,base);
	return simple_strtoul(cp,endp,base);
}


/******************************************************************************
 * usb command intepreter
 */
shellcmd xsh_usb(int argc, char * const argv[])
{

	int i;
	struct usb_device *dev = NULL;
	extern char usb_started;
#ifdef CONFIG_USB_STORAGE
	block_dev_desc_t *stor_dev;
#endif

	if ((strncmp(argv[1], "reset", 5) == 0) ||
			 (strncmp(argv[1], "start", 5) == 0)) {
			usb_stop();
			printf("(Re)start USB...\n");
			if (usb_init() >= 0) {
	#ifdef CONFIG_USB_STORAGE
				/* try to recognize storage devices immediately */
				usb_stor_curr_dev = usb_stor_scan(1);
	#endif
	#ifdef CONFIG_USB_HOST_ETHER
				/* try to recognize ethernet devices immediately */
				usb_ether_curr_dev = usb_host_eth_scan(1);
	#endif
	#ifdef CONFIG_USB_KEYBOARD
				drv_usb_kbd_init();
	#endif
			}
			return 0;
		}
	if (strncmp(argv[1], "stop", 4) == 0) {
#ifdef CONFIG_USB_KEYBOARD
		if (argc == 2) {
			if (usb_kbd_deregister() != 0) {
				printf("USB not stopped: usbkbd still"
					" using USB\n");
				return 1;
			}
		} else {
			/* forced stop, switch console in to serial */
			console_assign(stdin, "serial");
			usb_kbd_deregister();
		}
#endif
		printf("stopping USB..\n");
		usb_stop();
		return 0;
	}

	if (strncmp(argv[1], "help", 4) == 0) {
		help();
		return 0;
	}
	if (!usb_started) {
		printf("USB is stopped. Please issue 'usb start' first.\n");
		return 1;
	}
	if (strncmp(argv[1], "tree", 4) == 0) {
		kprintf("USB device tree:\n\r");
		for (i = 0; i < USB_MAX_DEVICE; i++) {
			dev = usb_get_dev_index(i);
			if (dev == NULL)
				break;
			if (dev->parent == NULL)
				usb_show_tree(dev);
		}
		return 0;
	}
	if (strncmp(argv[1], "inf", 3) == 0) {
		int d;
		if (argc == 2) {
			for (d = 0; d < USB_MAX_DEVICE; d++) {
				dev = usb_get_dev_index(d);
				if (dev == NULL)
					break;
				usb_display_desc(dev);
				usb_display_config(dev);
			}
			return 0;
		} else {
			int d;

			i = simple_strtoul(argv[2], NULL, 16);
			printf("config for device %d\n", i);
			for (d = 0; d < USB_MAX_DEVICE; d++) {
				dev = usb_get_dev_index(d);
				if (dev == NULL)
					break;
				if (dev->devnum == i)
					break;
			}
			if (dev == NULL) {
				printf("*** No device available ***\n");
				return 0;
			} else {
				usb_display_desc(dev);
				usb_display_config(dev);
			}
		}
		return 0;
	}
#ifdef CONFIG_USB_STORAGE
	if (strncmp(argv[1], "stor", 4) == 0)
		return usb_stor_info();
#if 0
	if (strncmp(argv[1], "part", 4) == 0) {

		int devno, ok = 0;
		if (argc == 2) {
			for (devno = 0; ; ++devno) {
				stor_dev = usb_stor_get_dev(devno);
				if (stor_dev == NULL)
					break;
				if (stor_dev->type != DEV_TYPE_UNKNOWN) {
					ok++;
					if (devno)
						printf("\n");
					USB_PRINTF("print_part of %x\n", devno);
					print_part(stor_dev);
				}
			}
		} else {
			devno = simple_strtoul(argv[2], NULL, 16);
			stor_dev = usb_stor_get_dev(devno);
			if (stor_dev != NULL &&
			    stor_dev->type != DEV_TYPE_UNKNOWN) {
				ok++;
				USB_PRINTF("print_part of %x\n", devno);
				print_part(stor_dev);

			}
		}

		if (!ok) {
			printf("\nno USB devices available\n");
			return 1;
		}

		return 0;
	}
#endif
	if (strncmp(argv[1], "read",4) == 0) {
		if (usb_stor_curr_dev < 0) {
			kprintf("no current device selected\n");
			return 1;
		}
		if (argc ==4) {
			unsigned long blk  = simple_strtoul(argv[2], NULL, 16);
			unsigned long cnt  = simple_strtoul(argv[3], NULL, 16);
			unsigned long n;

			kprintf("\nUSB read: device %d block # %d, count %d"
				" ... ", usb_stor_curr_dev, blk, cnt);
			stor_dev = usb_stor_get_dev(usb_stor_curr_dev);
			char buffer[stor_dev->blksz * cnt];
			n = stor_dev->block_read(usb_stor_curr_dev, blk, cnt,
						 &buffer);
			kprintf("%d blocks read: %s\n", n,
				(n == cnt) ? "OK" : "ERROR");

			if (n == cnt){
				int i = 0;
				kprintf("Data:\r\n");
				for(i = 0;i <50;i++){
					kprintf("%s",buffer[i]);
				}
				kprintf("\r\n");
				return 0;
			}
			return 1;


		} else {
			help();
			return 1;
		}


		if (argc == 5) {
			unsigned long addr = simple_strtoul(argv[2], NULL, 16);
			unsigned long blk  = simple_strtoul(argv[3], NULL, 16);
			unsigned long cnt  = simple_strtoul(argv[4], NULL, 16);
			unsigned long n;
			printf("\nUSB read: device %d block # %ld, count %ld"
				" ... ", usb_stor_curr_dev, blk, cnt);
			stor_dev = usb_stor_get_dev(usb_stor_curr_dev);
			n = stor_dev->block_read(usb_stor_curr_dev, blk, cnt,
						 (ulong *)addr);
			printf("%ld blocks read: %s\n", n,
				(n == cnt) ? "OK" : "ERROR");
			if (n == cnt)
				return 0;
			return 1;
		}
	}
	if (strncmp(argv[1], "write",5) == 0) {
		printf("disabled\r\n");
# if 0
		if (usb_stor_curr_dev < 0) {
			printf("no current device selected\n");
			return 1;
		}
		if (argc == 5) {
			unsigned long addr = simple_strtoul(argv[2], NULL, 16);
			unsigned long blk  = simple_strtoul(argv[3], NULL, 16);
			unsigned long cnt  = simple_strtoul(argv[4], NULL, 16);
			unsigned long n;
			printf("\nUSB write: device %d block # %ld, count %ld"
				" ... ", usb_stor_curr_dev, blk, cnt);
			stor_dev = usb_stor_get_dev(usb_stor_curr_dev);
			n = stor_dev->block_write(usb_stor_curr_dev, blk, cnt,
						(ulong *)addr);
			printf("%ld blocks write: %s\n", n,
				(n == cnt) ? "OK" : "ERROR");
			if (n == cnt)
				return 0;
			return 1;
		}
#endif
	}
	if (strncmp(argv[1], "dev", 3) == 0) {
		if (argc == 3) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);
			kprintf("\nUSB device %d: ", dev);
			stor_dev = usb_stor_get_dev(dev);
			if (stor_dev == NULL) {
				kprintf("unknown device\r\n");
				return 1;
			}
			kprintf("\nUSB Device %d: \r\n", dev);
			dev_print(stor_dev);
			if (stor_dev->type == DEV_TYPE_UNKNOWN)
				return 1;
			usb_stor_curr_dev = dev;
			kprintf("... is now current device\r\n");
			return 0;
		} else {
			kprintf("\n---Current Device---\r\nUSB device %d: \r\n", usb_stor_curr_dev);
			stor_dev = usb_stor_get_dev(usb_stor_curr_dev);
			dev_print(stor_dev);
			if (stor_dev->type == DEV_TYPE_UNKNOWN)
				return 1;
			return 0;
		}
		return 0;
	}
#endif /* CONFIG_USB_STORAGE */
		help();
	return 0;
}



#ifdef CONFIG_USB_STORAGE
/*U_BOOT_CMD(
	usb,	5,	1,	do_usb,
	"USB sub-system",
	"start - start (scan) USB controller\n"
	"usb reset - reset (rescan) USB controller\n"
	"usb stop [f] - stop USB [f]=force stop\n"
	"usb tree - show USB device tree\n"
	"usb info [dev] - show available USB devices\n"
	"usb storage - show details of USB storage devices\n"
	"usb dev [dev] - show or set current USB storage device\n"
	"usb part [dev] - print partition table of one or all USB storage"
	" devices\n"
	"usb read addr blk# cnt - read `cnt' blocks starting at block `blk#'\n"
	"    to memory address `addr'\n"
	"usb write addr blk# cnt - write `cnt' blocks starting at block `blk#'\n"
	"    from memory address `addr'"
);*/



#else
/*U_BOOT_CMD(
	usb,	5,	1,	do_usb,
	"USB sub-system",
	"start - start (scan) USB controller\n"
	"usb reset - reset (rescan) USB controller\n"
	"usb tree - show USB device tree\n"
	"usb info [dev] - show available USB devices"
);*/
#endif




