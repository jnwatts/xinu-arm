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
        usb_init();
    }

    return OK;
}
