/*
 * (C) Copyright 2012 Stephen Warren
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common2.h>
#include <io.h>
#include <mbox.h>
#include "asm/types.h"
#include <usleep.h>

#define TIMEOUT (100 * 1000) /* 100mS in uS */

int bcm2835_mbox_call_raw(u32 chan, u32 send, u32 *recv)
{
	struct bcm2835_mbox_regs *regs =
		(struct bcm2835_mbox_regs *)BCM2835_MBOX_PHYSADDR;
	//ulong endtime = get_timer(0) + TIMEOUT;
	u32 val;

	//debug("time: %lu timeout: %lu\n", get_timer(0), endtime);

	if (send & BCM2835_CHAN_MASK) {
		kprintf("mbox: Illegal mbox data 0x%08x\n", send);
		return -1;
	}

	/* Drain any stale responses */
#if 0
//old code
	for (;;) {
		val = readl(&regs->status);
		if (val & BCM2835_MBOX_STATUS_RD_EMPTY)
			break;
		if (get_timer(0) >= endtime) {
			kprintf("mbox: Timeout draining stale responses\n");
			return -1;
		}
		val = readl(&regs->read);
	}
#endif

	int wait = 0;
	for (;;) {
		val = readl(&regs->status);
		if (val & BCM2835_MBOX_STATUS_RD_EMPTY)
			break;
		if (wait >= TIMEOUT) {
			kprintf("mbox: Timeout draining stale responses\n");
			return -1;
		}
		val = readl(&regs->read);
		mdelay(200);
		++wait;
	}
	/* Wait for space to send */

#if 0
	for (;;) {
		val = readl(&regs->status);
		if (!(val & BCM2835_MBOX_STATUS_WR_FULL))
			break;
		if (get_timer(0) >= endtime) {
			kprintf("mbox: Timeout waiting for send space\n");
			return -1;
		}
	}
#endif
	wait = 0;
	for (;;) {
		val = readl(&regs->status);
		if (!(val & BCM2835_MBOX_STATUS_WR_FULL))
			break;
		if (wait >= TIMEOUT) {
			kprintf("mbox: Timeout waiting for send space\n");
			return -1;
		}
		mdelay(200);
		++wait;
	}

	/* Send the request */

	val = BCM2835_MBOX_PACK(chan, send);
	//debug("mbox: TX raw: 0x%08x\n", val);
	writel(val, &regs->write);

	/* Wait for the response */

	#if 0
	for (;;) {
		val = readl(&regs->status);
		if (!(val & BCM2835_MBOX_STATUS_RD_EMPTY))
			break;
		if (get_timer(0) >= endtime) {
			kprintf("mbox: Timeout waiting for response\n");
			return -1;
		}
	}
	#endif
	wait = 0;
	for (;;) {
		val = readl(&regs->status);
		if (!(val & BCM2835_MBOX_STATUS_RD_EMPTY))
			break;
		if (wait >= TIMEOUT) {
			kprintf("mbox: Timeout waiting for response\n");
			return -1;
		}
		mdelay(200);
		++wait;
	}
	/* Read the response */

	val = readl(&regs->read);
	//debug("mbox: RX raw: 0x%08x\n", val);

	/* Validate the response */

	if (BCM2835_MBOX_UNPACK_CHAN(val) != chan) {
		kprintf("mbox: Response channel mismatch\n");
		return -1;
	}

	*recv = BCM2835_MBOX_UNPACK_DATA(val);

	return 0;
}

#ifdef DEBUG
void dump_buf(struct bcm2835_mbox_hdr *buffer)
{
	u32 *p;
	u32 words;
	int i;

	p = (u32 *)buffer;
	words = buffer->buf_size / 4;
	for (i = 0; i < words; i++)
		kprintf("    0x%04x: 0x%08x\n", i * 4, p[i]);
}
#endif

int bcm2835_mbox_call_prop(u32 chan, struct bcm2835_mbox_hdr *buffer)
{
	int ret;
	u32 rbuffer;
	struct bcm2835_mbox_tag_hdr *tag;
	int tag_index;

#ifdef DEBUG
	kprintf("mbox: TX buffer\n");
	dump_buf(buffer);
#endif

	ret = bcm2835_mbox_call_raw(chan, (u32)buffer, &rbuffer);
	if (ret)
		return ret;
	if (rbuffer != (u32)buffer) {
		kprintf("mbox: Response buffer mismatch\n");
		return -1;
	}

#ifdef DEBUG
	kprintf("mbox: RX buffer\n");
	dump_buf(buffer);
#endif

	/* Validate overall response status */

	if (buffer->code != BCM2835_MBOX_RESP_CODE_SUCCESS) {
		kprintf("mbox: Header response code invalid\n");
		return -1;
	}

	/* Validate each tag's response status */

	tag = (void *)(buffer + 1);
	tag_index = 0;
	while (tag->tag) {
		if (!(tag->val_len & BCM2835_MBOX_TAG_VAL_LEN_RESPONSE)) {
			kprintf("mbox: Tag %d missing val_len response bit\n",
				tag_index);
			return -1;
		}
		/*
		 * Clear the reponse bit so clients can just look right at the
		 * length field without extra processing
		 */
		tag->val_len &= ~BCM2835_MBOX_TAG_VAL_LEN_RESPONSE;
		tag = (void *)(((u8 *)tag) + sizeof(*tag) + tag->val_buf_size);
		tag_index++;
	}

	return 0;
}
