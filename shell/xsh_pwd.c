/**
 * @file     xsh_clear.c
 * @provides xsh_clear.
 *
 * $Id: xsh_clear.c 2065 2009-09-04 21:44:36Z brylow $
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/**
 * Shell command (cd) changes the current working directory.
 * @param nargs number of arguments
 * @param args  array of arguments
 * @return non-zero value on error
 */
shellcmd xsh_pwd(int nargs, char *args[])
{
    /* Check for correct number of arguments */
    if (nargs > 1)
    {
        fprintf(stderr, "%s: too many arguments\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information.\n",
                args[0]);
        return 1;
    }

    printf("%s\n", thrtab[thrcurrent].currdir);

    return 0;
}
