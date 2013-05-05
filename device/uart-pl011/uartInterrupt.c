/**
 * @file uartInterrupt.c
 * @provides uartInterrupt.
 *
 * $Id: uartInterrupt.c 2102 2009-10-26 20:36:13Z brylow $
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <thread.h>
#include <device.h>
#include <uart.h>
#include <interrupt.h>
#include "pl011.h"

extern int resdefer;

/**
 * Decode hardware interrupt request from UART device.
 */
interrupt uartInterrupt(void)
{
    //int u = 0, iir = 0, lsr = 0, count = 0;
    int u = 0, ris = 0, lsr = 0, count = 0;
    char c;
    struct uart *uartptr = NULL;
    struct uart_csreg *regptr = NULL;

    resdefer = 1;               /* deferral rescheduling. */

    for (u = 0; u < NUART; u++)
    {
        uartptr = &uarttab[u];
        if (NULL == uartptr) //if no UART
        {
            continue;
        }
        regptr = (struct uart_csreg *)uartptr->csr;
        if (NULL == regptr) //if no register pointer
        {
            continue;
        }

        /* Check raw interrupt status register */
        ris = regptr->ris;
        if (ris == 0) //if there is no interrupt
        {
            continue;
        }

        //handle whatever interrupt occurred
        if(ris & PL011_RIS_TXRIS){ //if the transmitter FIFO ran out
            uartptr->oirq++; //increment output IRQ count
            regptr->icr |= PL011_ICR_TXIC; //clear transmitter interrupt
	     /*signal(uartptr->osema);*/
	     /*printf("semacount: %d\n", semcount(uartptr->osema));*/
	     /*printf("ocount: 0x%8X\n", (*uartptr).ocount);*/
            count = 0;
            if (uartptr->ocount > 0) //if we have bytes in the buffer
            {
                /* Write characters to the lower half of the UART. */
                do
                {
                    count++;
                    uartptr->ocount--;
                    regptr->buffer = uartptr->out[uartptr->ostart]; //write a character to the FIFO
                    uartptr->ostart = (uartptr->ostart + 1) % UART_OBLEN;
                }
                while ((count < PL011_FIFO_LEN) && (uartptr->ocount > 0));
            }

            if (count)
            {
                uartptr->cout += count;
		  /*printf("signaling: %d\n",count);*/
		  /*signaln(uartptr->osema,count);*/
		  /*signaln(uartptr->osema, count);*/
            }
            /* If no characters were written, set the output idle flag. */
            else
            {
                uartptr->oidle = TRUE;
		  
		  
            }
		/*printf("count: %d\n", count);*/
	      /*printf("signaling: %d\n",count);*/
            
        }else if(ris & PL011_RIS_RXRIS){ //if the receiver FIFO is full
            uartptr->iirq++; //increment input IRQ count
            count = 0;
            while ((regptr->fr & PL011_FR_RXFE) == 0) //while the receive FIFO is not empty
            {
                c = regptr->buffer; //get a character from the FIFO
                if (uartptr->icount < UART_IBLEN)
                {
                    uartptr->in
                        [(uartptr->istart +
                          uartptr->icount) % UART_IBLEN] = c;
                    uartptr->icount++;
                    count++;
                }
                else
                {
                    uartptr->ovrrn++;
                }
            }
            uartptr->cin += count;
	     /*printf("inputsema signal: %d\n", count);*/
            signaln(uartptr->isema, count);
        }
    }

#ifdef FLUKE_ARM
    // Tell the VIC that the interrupt was handled.
    irq_handled();
#endif

    if (--resdefer > 0)
    {
        resdefer = 0;
        resched();
    }
}
